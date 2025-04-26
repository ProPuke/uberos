#pragma once

#include "Timer.hpp"

#include <kernel/drivers.hpp>

namespace driver {
	inline auto Timer::find_and_claim_timer(Callback onTerminated, void *onTerminatedData) -> Try<ClaimedTimer> {
		auto timer = drivers::find_and_activate<driver::Timer>();
		if(!timer) return Failure{"No timers available"};

		// does the default activated timer have any claimables?
		if(auto timerId = timer->claim_timer(); timerId) {
			return ClaimedTimer{DriverReference<Timer>{timer, onTerminated, onTerminatedData}, timerId.result};
		}

		// ...failing that, check ALL active drivers
		for(auto &timer:drivers::iterate<driver::Timer>()){
			if(timer.api.is_active()){
				if(auto timerId = timer.claim_timer(); timerId) {
					return ClaimedTimer{DriverReference<Timer>{&timer, onTerminated, onTerminatedData}, timerId.result};
				}
			}
		}

		// ...failing that, check ALL drivers, activating if not
		for(auto &timer:drivers::iterate<driver::Timer>()){
			if(drivers::start_driver(timer)){
				if(auto timerId = timer.claim_timer(); timerId) {
					return ClaimedTimer{DriverReference<Timer>{&timer, onTerminated, onTerminatedData}, timerId.result};
				}
			}
		}

		// nothing found ¯\_(ツ)_/¯
		return Failure{"No timers available"};
	}

	inline auto Timer::claim_timer() -> Try<U8> {
		auto id = timersInUse.get_first_false();
		if(id==~0) return Failure{"No timers available"};

		timersInUse.set(id, true);

		return id;
	}

	inline void Timer::release_timer(U8 id) {
		timersInUse.set(id, false);
	}

	inline /**/ TimerQueue::TimerQueue(Timer &timer):
		timer(timer)
	{}

	inline void TimerQueue::_on_schedule_timer(void *_instance){
		auto self = (TimerQueue*)_instance;

		// const auto time = self->timer.now64();
		const auto time = self->_nextScheduledTime; // we use the stored time, rather than rely on the actual clock. Timings can drift, so use what we EXPECTED the time to be

		{
			auto count = 0u;
			while(count<self->scheduledImportantCallbacks.length&&self->scheduledImportantCallbacks[count].time+-time>0xffffffff<<2){
				self->scheduledImportantCallbacks[count++].time = 0; // we count and initially set the callbacks to 0, so that any new additions from the callbacks position after them
			}

			for(auto i=0u;i<count;i++){
				//NOTE: None of these callbacks should erase from scheduledImportantCallbacks. This should be done purely below with the shift_left below
				self->scheduledImportantCallbacks[i].callback(self->scheduledImportantCallbacks[i].data, self->scheduledImportantCallbacks[i].id);
			}

			self->scheduledImportantCallbacks.shift_left(count);
		}

		{
			auto count = 0u;
			while(count<self->scheduledCallbacks.length&&self->scheduledCallbacks[count].time+-time>0xffffffff<<2){
				self->scheduledCallbacks[count++].time = 0; // we count and initially set the callbacks to 0, so that any new additions from the callbacks position after them
			}

			for(auto i=0u;i<count;i++){
				//NOTE: None of these callbacks should erase from scheduledCallbacks. This should be done purely below with the shift_left below
				self->scheduledCallbacks[i].callback(self->scheduledCallbacks[i].data, self->scheduledCallbacks[i].id);
			}

			self->scheduledCallbacks.shift_left(count);
		}

		if(self->scheduledCallbacks.length>0){
			if(self->scheduledImportantCallbacks.length>0){
				self->timer.set_timer(0, self->_nextScheduledTime = min(self->scheduledCallbacks[0].time, self->scheduledImportantCallbacks[0].time), _on_schedule_timer, self);

			}else{
				self->timer.set_timer(0, self->_nextScheduledTime = self->scheduledCallbacks[0].time, _on_schedule_timer, self);
			}

		}else if(self->scheduledImportantCallbacks.length>0){
			self->timer.set_timer(0, self->_nextScheduledTime = self->scheduledImportantCallbacks[0].time, _on_schedule_timer, self);
		}
	}

	inline auto TimerQueue::schedule(U32 usecs, Timer::ScheduledCallback callback, void *data) -> U32 {
		auto time = timer.now64()+usecs;

		for(auto i=0u;i<scheduledCallbacks.length;i++){
			if(!scheduledCallbacks[i].time) continue; // don't insert before zero times

			auto diff = time - scheduledCallbacks[i].time;
			if(diff>0xffffffff<<2){ // if time LOWER than existing (this will cause wrap-around and so a large value)
				scheduledCallbacks.insert(i, time, callback, nextTimerId, data);
				if(i==0){
					if(scheduledImportantCallbacks.length>0 && scheduledImportantCallbacks[0].time>0){
						timer.set_timer(0, _nextScheduledTime = min(time, scheduledImportantCallbacks[0].time), _on_schedule_timer, this);
					}else{
						timer.set_timer(0, _nextScheduledTime = time, _on_schedule_timer, this);
					}
				}
				return nextTimerId++;
			}
		}

		scheduledCallbacks.push_back(time, callback, nextTimerId, data);
		return nextTimerId++;
	}

	inline auto TimerQueue::schedule_important(U32 usecs, Timer::ScheduledCallback callback, void *data) -> U32 {
		auto time = timer.now64()+usecs;

		for(auto i=0u;i<scheduledImportantCallbacks.length;i++){
			if(!scheduledImportantCallbacks[i].time) continue; // don't insert before zero times

			auto diff = time - scheduledImportantCallbacks[i].time;
			if(diff>0xffffffff<<2){ // if time LOWER than existing (this will cause wrap-around and so a large value)
				scheduledImportantCallbacks.insert(i, time, callback, nextTimerId, data);
				if(i==0){
					if(scheduledCallbacks.length>0 && scheduledCallbacks[0].time>0){
						timer.set_timer(0, _nextScheduledTime = min(time, scheduledCallbacks[0].time), _on_schedule_timer, this);
					}else{
						timer.set_timer(0, _nextScheduledTime = time, _on_schedule_timer, this);
					}
				}
				return nextTimerId++;
			}
		}

		scheduledImportantCallbacks.push_back(time, callback, nextTimerId, data);
		return nextTimerId++;
	}
}
