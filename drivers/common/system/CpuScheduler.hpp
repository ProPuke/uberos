#pragma once

#include <drivers/Scheduler.hpp>
#include <drivers/Timer.hpp>

#include <kernel/Process.hpp>
#include <kernel/Thread.hpp>

#include <common/LList.hpp>

namespace driver::system {
	struct CpuScheduler: Scheduler {
		DRIVER_INSTANCE(CpuScheduler, 0xfe8dac43, "cpuSched", "CPU Scheduler", Scheduler)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override { return {"Thread scheduler drivers cannot be stopped"}; };

		auto can_stop_driver() -> bool override { return false; }
		auto can_restart_driver() -> bool override { return false; }

		void add_thread(Thread&) override;
		void remove_thread(Thread&) override;
		auto get_current_thread() -> Thread* override;
		void yield() override;

		void _on_thread_sleep(Thread&, U32 usecs) override;
		void _on_thread_paused(Thread&) override;
		void _on_thread_paused_resumed(Thread&) override;
		void _on_thread_sleeping_resumed(Thread&) override;
		void _on_thread_terminated(Thread&) override;
		void _on_thread_priorities_changed(Thread&) override;

	protected:

		driver::Timer::ClaimedTimer timer;
		Process kernelProcess{"Kernel"};
		Thread *kernelThread;
		// Process idleProcess{"Idle Process"};
		// Thread *idleThread;

		// in usecs
		U32 minInterval     =   1'000;
		U32 averageInterval =  30'000;
		U32 maxInterval     = 100'000;

		U32 totalActivePriority = 0;

		Thread *currentThread = nullptr;

		LList<Thread> activeThreads;
		LList<Thread> sleepingThreads;
		LList<Thread> pausedThreads;
		LList<Thread> terminatedThreads;

		void _yield();

		static void _on_yield_timeout(void *_scheduler);
	};
}
