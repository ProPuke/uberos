#include "timer.hpp"

#include <kernel/exceptions.hpp>
#include <kernel/PodArray.hpp>

namespace timer {
	struct ScheduledCallback {
		U64 time;
		void(*callback)(void *data);
		void *data;
	};

	PodArray<ScheduledCallback> scheduledCallbacks;
	PodArray<ScheduledCallback> scheduledImportantCallbacks;

	void _schedule_update(U32 usecs);

	void schedule(U32 usecs, void(*callback)(void *data), void *data) {
		const auto time = now64()+usecs;

		for(auto i=0u;i<scheduledCallbacks.length;i++){
			auto diff = time - scheduledCallbacks[i].time;
			if(diff>0&&diff<0xffffffff<<2){
				scheduledCallbacks.insert(i, time, callback, data);
				if(i==0){
					_schedule_update(usecs);
				}
				return;
			}
		}

		scheduledCallbacks.push_back(time, callback, data);
	}

	void schedule_important(U32 usecs, void(*callback)(void *data), void *data) {
		const auto time = now64()+usecs;

		for(auto i=0u;i<scheduledImportantCallbacks.length;i++){
			auto diff = time - scheduledImportantCallbacks[i].time;
			if(diff>0&&diff<0xffffffff<<2){
				scheduledImportantCallbacks.insert(i, time, callback, data);
				if(i==0){
					_schedule_update(usecs);
				}
				return;
			}
		}

		scheduledImportantCallbacks.push_back(time, callback, data);
	}

	bool _check() {
		exceptions::Guard guard;

		static bool inCheck = false;
		if(inCheck) return false;

		inCheck = true;

		const auto time = now();

		{
			auto count = 0u;
			while(count<scheduledImportantCallbacks.length&&scheduledImportantCallbacks[count].time-time<0xffffffff<<2){
				scheduledImportantCallbacks[count++].time = 0; // we count and initially set the callbacks to 0, so that any new additions from the callbacks position after them
			}

			for(auto i=0u;i<count;i++){
				//NOTE: None of these callbacks should erase from scheduledImportantCallbacks. This should done purely below with the shift_left below
				scheduledImportantCallbacks[i].callback(scheduledImportantCallbacks[i].data);
			}


			scheduledImportantCallbacks.shift_left(count);
		}

		{
			auto count = 0u;
			while(count<scheduledCallbacks.length&&scheduledCallbacks[count].time-time<0xffffffff<<2){
				scheduledCallbacks[count++].time = 0; // we count and initially set the callbacks to 0, so that any new additions from the callbacks position after them
			}

			for(auto i=0u;i<count;i++){
				//NOTE: None of these callbacks should erase from scheduledCallbacks. This should done purely below with the shift_left below
				scheduledCallbacks[i].callback(scheduledCallbacks[i].data);
			}


			scheduledCallbacks.shift_left(count);
		}

		if(scheduledImportantCallbacks.length>0){
			const auto next = scheduledCallbacks.length>0?min(scheduledImportantCallbacks[0].time, scheduledCallbacks[0].time):scheduledImportantCallbacks[0].time;
			_schedule_update(next>time?next-time:0);

		}else if(scheduledCallbacks.length>0){
			const auto next = scheduledCallbacks[0].time;
			_schedule_update(next>time?next-time:0);
		}

		inCheck = false;

		return true;
	}
}
