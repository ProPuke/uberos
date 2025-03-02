#include "Thread.hpp"

// #if defined(ARCH_ARM32)
// 	#include <kernel/arch/arm/scheduler.hpp>
// #endif

#include <kernel/Log.hpp>
#include <kernel/time.hpp>
#include <kernel/memory.hpp>

static Log log("thread");

/**/ Thread:: Thread(Process &process):
	process(process)
{}

/**/ Thread::~Thread() {
	terminate();

	memory::Transaction().free_page(*stackPage);
}

void Thread::sleep(U32 usecs) {
	if(state!=State::active||!scheduler) return;

	state = State::sleeping;

	if(scheduler){
		scheduler->_on_thread_sleep(*this, usecs);
	}
}

void Thread::pause() {
	if(state!=State::active) return;

	//TODO:save state on next schedule and move it into a paused state *then* ?

	state = State::paused;

	if(scheduler){
		scheduler->_on_thread_paused(*this);
	}
}

void Thread::resume() {
	switch(state){
		case State::active:
		break;
		case State::sleeping:
			state = State::active;

			if(scheduler){
				scheduler->_on_thread_sleeping_resumed(*this);
			}
		break;
		case State::paused:
			state = State::active;

			if(scheduler){
				scheduler->_on_thread_paused_resumed(*this);
			}
		break;
		case State::terminated:
		break;
	}
}

void Thread::terminate() {
	if(state==State::terminated) return;

	state = State::terminated;

	if(scheduler){
		scheduler->_on_thread_terminated(*this);
	}

	events.trigger({
		type: Event::Type::terminated
	});
}

void Thread::set_priority(U16 set) {
	if(priority==set) return;

	priority = set;

	if(scheduler){
		scheduler->_on_thread_priorities_changed(*this);
	}
}
