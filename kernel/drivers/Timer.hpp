#pragma once

#include <kernel/drivers/Hardware.hpp>

#include <common/Bitmask.hpp>

namespace driver {
	struct Timer: Hardware {
		DRIVER_TYPE(Timer, "timer", "Hardware Timer Driver", Hardware)

		typedef void (*ScheduledCallback)(void *data, U32 id);
		typedef void (*ScheduledCallback2)(void *data);
		typedef void (*ScheduledCallback3)();

		typedef void (*Callback)(void *data);
		typedef void (*Callback2)();

		virtual auto now() -> U32 = 0;
		virtual auto now64() -> U64 = 0;
		virtual auto schedule(U32 usecs, ScheduledCallback, void *data) -> U32 = 0;
		virtual auto schedule_important(U32 usecs, ScheduledCallback, void *data) -> U32 = 0;
		inline auto schedule(U32 usecs, ScheduledCallback2 callback, void *data) -> U32 { return schedule(usecs, (ScheduledCallback)callback, data); }
		inline auto schedule_important(U32 usecs, ScheduledCallback2 callback, void *data) -> U32 { return schedule_important(usecs, (ScheduledCallback)callback, data); }
		inline auto schedule(U32 usecs, ScheduledCallback3 callback) -> U32 { return schedule(usecs, (ScheduledCallback)callback, nullptr); }
		inline auto schedule_important(U32 usecs, ScheduledCallback3 callback) -> U32 { return schedule_important(usecs, (ScheduledCallback)callback, nullptr); }

		virtual auto get_timer_count() -> U8 = 0;
		virtual void set_timer(U8, U32 usecs, Callback, void *data) = 0;
		inline void set_timer(U8 id, U32 usecs, Callback2 callback) { return set_timer(id, usecs, (Callback)callback, nullptr); }
		virtual void stop_timer(U8) = 0;

		auto claim_timer() -> Try<U8>;
		void release_timer(U8);

		struct ClaimedTimer {
			DriverReference<Timer> timer;
			U8 timerId;

			void set(U32 usecs, Callback callback, void *data) { return timer->set_timer(timerId, usecs, callback, data); }
			void set(U32 usecs, Callback2 callback) { return timer->set_timer(timerId, usecs, callback); }
			void stop() { return timer->stop_timer(timerId); }
		};

		static auto find_and_claim_timer(Callback onTerminated, void *onTerminatedData) -> Try<ClaimedTimer>;

	protected:
		Bitmask256 timersInUse;
	};
}

#include <kernel/PodArray.hpp>

namespace driver {
	struct TimerQueue {
		struct Scheduled {
			U64 time;
			Timer::ScheduledCallback callback;
			U32 id;
			void *data;
		};

		/**/ TimerQueue(Timer &timer);
		Timer &timer;
		U32 nextTimerId = 0;

		PodArray<Scheduled> scheduledCallbacks;
		PodArray<Scheduled> scheduledImportantCallbacks;

		U64 _nextScheduledTime;

		static void _on_schedule_timer(void *_instance);

		auto schedule(U32 usecs, Timer::ScheduledCallback callback, void *data) -> U32;
		auto schedule_important(U32 usecs, Timer::ScheduledCallback callback, void *data) -> U32;
	};
}

#include "Timer.inl"
