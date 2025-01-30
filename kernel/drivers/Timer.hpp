#pragma once

#include <kernel/drivers/Hardware.hpp>

namespace driver {
	struct Timer: Hardware {
		DRIVER_TYPE(Timer, "timer", "Hardware Timer Driver", Hardware)

		typedef void (*Callback)(void *data);

		virtual auto now() -> U32 = 0;
		virtual auto now64() -> U64 = 0;
		virtual void schedule(U32 usecs, Callback, void *data) = 0;
		virtual void schedule_important(U32 usecs, Callback, void *data) = 0;
		inline void schedule(U32 usecs, void(*callback)()) { schedule(usecs, (Callback)callback, nullptr); }
		inline void schedule_important(U32 usecs, void(*callback)()) { schedule_important(usecs, (Callback)callback, nullptr); }

		virtual auto get_timer_count() -> U8 = 0;
		virtual void set_timer(U8, U32 usecs, Callback, void *data) = 0;
		virtual void stop_timer(U8) = 0;
	};
}

#include <kernel/PodArray.hpp>

namespace driver {
	struct TimerQueue {
		struct Scheduled {
			U64 time;
			Timer::Callback callback;
			void *data;
		};

		/**/ TimerQueue(Timer &timer);
		Timer &timer;

		PodArray<Scheduled> scheduledCallbacks;
		PodArray<Scheduled> scheduledImportantCallbacks;

		U64 _nextScheduledTime;

		static void _on_schedule_timer(void *_instance);

		void schedule(U32 usecs, Timer::Callback callback, void *data);
		void schedule_important(U32 usecs, Timer::Callback callback, void *data);
	};
}

#include "Timer.inl"
