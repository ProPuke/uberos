#pragma once

#include <kernel/drivers/Software.hpp>

struct Thread;

namespace driver {
	struct Scheduler: Software {
		DRIVER_TYPE(Scheduler, "scheduler", "Thread Scheduler", Software)

		virtual void add_thread(Thread&) = 0;
		virtual void remove_thread(Thread&) = 0;
		virtual auto get_current_thread() -> Thread* = 0;
		virtual void yield() = 0;

		virtual void _on_thread_sleep(Thread&, U32 usecs) = 0;
		virtual void _on_thread_paused(Thread&) = 0;
		virtual void _on_thread_paused_resumed(Thread&) = 0;
		virtual void _on_thread_sleeping_resumed(Thread&) = 0;
		virtual void _on_thread_terminated(Thread&) = 0;
		virtual void _on_thread_priorities_changed(Thread&) = 0;
	};
}
