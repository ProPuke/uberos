#include "Thread.hpp"

#include "arch/arm/scheduler.hpp"
#include "Spinlock.hpp"
#include "timer.hpp"
#include "memory.hpp"

namespace scheduler {
	using namespace scheduler::arch::arm;
}

namespace thread {
	std::atomic<Thread*> currentThread = nullptr;

	LList<Thread> activeThreads;
	LList<Thread> sleepingThreads;
	LList<Thread> pausedThreads;
	LList<Thread> freedThreads;

	void _cleanup_thread() {
		{
			scheduler::Guard guard;

			stdio::print_info("_cleanup_thread");
			// Spinlock_Guard lock{scheduler::threadLock, __FUNCTION__};

			auto &thread = *currentThread.load();

			for(auto callback=thread.on_deleted.head; callback; callback=callback->next){
				callback->call(callback->data);
			}

			if(activeThreads.contains(thread)){
				activeThreads.pop(thread);
			}else if(sleepingThreads.contains(thread)){
				sleepingThreads.pop(thread);
			}else if(pausedThreads.contains(thread)){
				pausedThreads.pop(thread);
			}else if(freedThreads.contains(thread)){
				freedThreads.pop(thread);
			}else{
				stdio::print_error("Error: Terminating thread not found in any lists");
			}

			memory::Transaction().free_page(*thread.stackPage);
		}
		
		scheduler::yield();
	}
}

/**/ Thread::Thread(Process &process):
	process(process)
{}

void Thread::sleep(U64 usecs) {
	if(state!=State::active) return;

	{
		Spinlock_Guard lock(scheduler::threadLock, __FUNCTION__);

		state = State::sleeping;
		sleep_wake_time = timer::now() + usecs;

		thread::activeThreads.pop(*this);

		auto beforeThread = thread::sleepingThreads.head;
		while(beforeThread&&beforeThread->sleep_wake_time<=sleep_wake_time) beforeThread = beforeThread->next;

		if(beforeThread){
			thread::sleepingThreads.insert_before(*beforeThread, *this);
		}else{
			thread::sleepingThreads.push_back(*this);
		}
	}

	scheduler::yield();
}

void Thread::pause() {
	if(state!=State::active) return;

	//TODO:save state on next schedule and move it into a paused state *then* ?

	Spinlock_Guard lock(scheduler::threadLock, __FUNCTION__);

	thread::activeThreads.pop(*this);
	state = State::paused;
	thread::pausedThreads.push_back(*this);
}

void Thread::resume() {
	if(state!=State::paused) return;

	Spinlock_Guard lock(scheduler::threadLock, __FUNCTION__);

	thread::pausedThreads.pop(*this);
	state = State::active;
	thread::activeThreads.push_back(*this);
}

void Thread::terminate() {
	if(thread::currentThread==this) {
		thread::currentThread = nullptr;
	}

	//FIXME:thread might still be activeThreads regardless of state (some switch lists late)

	Spinlock_Guard lock(scheduler::threadLock, __FUNCTION__);

	switch(state){
		case State::active:
			thread::activeThreads.pop(*this);
		break;
		case State::sleeping:
			thread::sleepingThreads.pop(*this);
		break;
		case State::paused:
			thread::pausedThreads.pop(*this);
		break;
		case State::terminated:
			return;
		break;
	}

	state = State::terminated;
	thread::freedThreads.push_back(*this);
}
