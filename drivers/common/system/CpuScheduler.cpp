#include "CpuScheduler.hpp"

#include <kernel/Thread.hpp>

namespace driver::system {
	auto CpuScheduler::_on_start() -> Try<> {
		timer = TRY_RESULT(driver::Timer::find_and_claim_timer([](void *_scheduler) {
			auto &scheduler = *(CpuScheduler*)_scheduler;
			scheduler.api.fail_driver("Timer dropped");
		}, this));

		kernelThread = kernelProcess.create_kernel_thread([](){});

		idleThread = idleProcess.create_kernel_thread([](){
			while(true) asm("hlt");
		});

		currentThread = kernelThread;

		return {};
	}

	void CpuScheduler::add_thread(Thread &thread) {
		switch(thread.state){
			case Thread::State::active:
				totalActivePriority += thread.priority * thread.process.priority;
				activeThreads.push_back(thread);
			break;
			case Thread::State::paused:
				pausedThreads.push_back(thread);
			break;
			case Thread::State::sleeping:
				sleepingThreads.push_back(thread);
			break;
			case Thread::State::terminated:
				terminatedThreads.push_back(thread);
			break;
		}
	}

	void CpuScheduler::remove_thread(Thread &thread) {
		if(terminatedThreads.contains(thread)){
			terminatedThreads.pop(thread);

		}else if(activeThreads.contains(thread)){
			totalActivePriority -= thread.priority * thread.process.priority;
			activeThreads.pop(thread);

		}else if(sleepingThreads.contains(thread)){
			sleepingThreads.pop(thread);

		}else if(pausedThreads.contains(thread)){
			pausedThreads.pop(thread);
		}
	}

	auto CpuScheduler::get_current_thread() -> Thread* {
		return currentThread;
	}

	void CpuScheduler::yield() {
		timer.stop();

		_yield();
	}

	// yield() without a timer clear (not needed when called direct _from_ a timeout)
	void CpuScheduler::_yield() {
		if(currentThread&&currentThread!=idleThread){
			switch(currentThread->state){
				case Thread::State::active:
					activeThreads.pop(*currentThread);
					activeThreads.push_back(*currentThread);
				break;
				case Thread::State::sleeping:
				case Thread::State::paused:
				case Thread::State::terminated:
				break;
			}
		}

		auto oldThread = currentThread;
		currentThread = activeThreads.head;

		if(currentThread==oldThread) {
			timer.set(maxInterval, _on_yield_timeout, this);
			return;
		}

		if(currentThread){
			const auto threadPriority = currentThread->priority * currentThread->process.priority;

			// set to the average timeout interval, scaled by the ratio of this thread's priority vs all other queued priorities
			const auto maxTime = maths::clamp(averageInterval * activeThreads.size * threadPriority / totalActivePriority, minInterval, maxInterval);

			timer.set(maxTime, _on_yield_timeout, this);

			Thread::swap_state(*oldThread, *currentThread);

		}else{
			currentThread = idleThread;

			timer.set(maxInterval, _on_yield_timeout, this);

			Thread::swap_state(*oldThread, *currentThread);
		}
	}

	void CpuScheduler::_on_yield_timeout(void *_scheduler) {
		auto &scheduler = *(CpuScheduler*)_scheduler;

		scheduler._yield();
	}

	void CpuScheduler::_on_thread_sleep(Thread &thread, U32 usecs) {
		activeThreads.pop(thread);
		sleepingThreads.push_back(thread);

		// putting a thread to sleep ALWAYS sets the pending time id, which means this callback is always valid if the thread is still sleeping and has this id
		thread._pending_timer_id = timer.timer->schedule(usecs, [](void *_thread, U32 timerId){
			auto &thread = *(Thread*)_thread;

			if(thread.state!=Thread::State::sleeping) return;
			if(timerId!=thread._pending_timer_id) return;

			thread.resume();
		}, &thread);
	}

	void CpuScheduler::_on_thread_paused(Thread &thread) {
		activeThreads.pop(thread);
		pausedThreads.push_back(thread);
	}

	void CpuScheduler::_on_thread_paused_resumed(Thread &thread) {
		pausedThreads.pop(thread);
		activeThreads.push_back(thread);
	}

	void CpuScheduler::_on_thread_sleeping_resumed(Thread &thread) {
		sleepingThreads.pop(thread);
		activeThreads.push_back(thread);
	}

	void CpuScheduler::_on_thread_terminated(Thread &thread) {
		if(&thread==currentThread){
			//TODO
		}
		
		switch(currentThread->state){
			case Thread::State::active:
				activeThreads.pop(thread);
			break;
			case Thread::State::sleeping:
				sleepingThreads.pop(thread);
			break;
			case Thread::State::paused:
				pausedThreads.pop(thread);
			break;
			case Thread::State::terminated:				
				terminatedThreads.pop(thread);
			break;
		}

		terminatedThreads.push_back(thread);
	}

	void CpuScheduler::_on_thread_priorities_changed(Thread &thread) {
		totalActivePriority = 0;

		for(auto thread=activeThreads.head; thread; thread=thread->next){
			auto priority = thread->priority * thread->process.priority;
			totalActivePriority += priority;
		}
	}
}
