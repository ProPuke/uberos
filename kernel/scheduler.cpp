#include "scheduler.hpp"

#include <kernel/kernel.h>
#include <kernel/Log.hpp>
#include <kernel/memory.hpp>
#include <kernel/PodArray.hpp>
#include <kernel/Spinlock.hpp>
#include <kernel/Thread.hpp>
#include <kernel/timer.hpp>

static Log log("scheduler");

namespace thread {
	extern Spinlock threadLock;

	extern LList<Thread> activeThreads;
	extern LList<Thread> sleepingThreads;
	extern LList<Thread> pausedThreads;
	extern LList<Thread> freedThreads;
}

namespace scheduler {
	Spinlock spinlock("scheduler");

	const U32 defaultInterval = 30000; //30ms

	U32 currentInterval = defaultInterval;
	U32 lockDepth = 0;
	U32 deferredYields = 0;
	U32 lastSchedule = 0;

	PodArray<U32> resolutionRequirements;

	void init() {
		auto section = log.section("init...");

		auto mainProcess = new Process("kernel");
		auto &kernelStack = memory::Transaction().get_memory_page(memory::stack);
		auto mainThread = mainProcess->create_current_thread(kernelStack, KERNEL_STACK_SIZE);

		thread::activeThreads.push_back(*mainThread);
		::thread::currentThread = mainThread;

		timer::schedule_important(currentInterval, yield);
	}

	void push_resolution_requirement(U32 requirement) {
		Spinlock_Guard guard(spinlock);
		if(requirement<currentInterval) currentInterval = requirement;

		resolutionRequirements.push_back(requirement);
	}

	void pop_resolution_requirement() {
		const auto previous = resolutionRequirements.back();
		resolutionRequirements.remove_back();

		if(currentInterval==previous){
			// find lowest interval requirement
			currentInterval = defaultInterval;
			for(auto &requirement:resolutionRequirements){
				if(requirement<currentInterval) currentInterval = requirement;
			}
		}
	}

	void yield() {
		spinlock.lock();

		if(lockDepth>0){
			deferredYields++;
			spinlock.unlock();
			return;
		}

		thread::threadLock.lock();

		auto now = timer::now();
		lastSchedule = now;

		auto oldThread = ::thread::currentThread.load();

		//TODO: if oldThread is now sleeping, and still had more than the minimum epoch of time remaining, then record the extra remaining time it is owed

		// wake sleeping threads
		//TODO: ensure waking threads when recorded extra time bump up to the front of the queue, with a delay equal to the remaining time, otherwise schedule is usual
		for(Thread *thread = thread::sleepingThreads.head; thread && now >= thread->sleep_wake_time || now < thread->sleep_start_time; thread = thread::sleepingThreads.head) {
			thread->state = Thread::State::active;
			thread::sleepingThreads.pop_front();
			thread::activeThreads.push_front(*thread);
		}

		if(thread::activeThreads.size==0||thread::activeThreads.size==1&&thread::activeThreads.head==oldThread){
			//if no other threads to switch to, we can ease up on the scheduling a bit..

			timer::schedule_important(max(currentInterval*4, currentInterval*oldThread->scheduler_timeslice_percentage/100), yield);
			thread::threadLock.unlock();
			spinlock.unlock();
			return;

		}else{
			// if the current thread has been scheduled again, then put it at back of the queue and we're done. Nothing to swap
			if(thread::activeThreads.head==oldThread){
				thread::activeThreads.push_back(*thread::activeThreads.pop_front());

				timer::schedule_important(currentInterval*oldThread->scheduler_timeslice_percentage/100, yield);

				thread::threadLock.unlock();
				spinlock.unlock();
				return;
			}

			auto &newThread = *thread::activeThreads.pop_front();

			if(newThread.state==Thread::State::active){
				thread::activeThreads.push_back(newThread);
			}

			::thread::currentThread = &newThread;
			auto thread_timeslice = currentInterval*newThread.scheduler_timeslice_percentage/100;

			timer::schedule_important(thread_timeslice, yield);

			deferredYields = 0; //nothing was missed as we've just left, so do not auto fire any on unlock()

			thread::threadLock.unlock();
			spinlock.unlock();

			Thread::swap_state(*oldThread, newThread);
		}
	}
	
	void lock() {
		Spinlock_Guard guard(spinlock);
		lockDepth++;
	}

	void unlock() {
		spinlock.lock();
		lockDepth--;

		if(lockDepth==0){
			if(deferredYields>0){
				deferredYields = 0;
				spinlock.unlock();
				yield();
				return;
			}
		}

		spinlock.unlock();
	}
}
