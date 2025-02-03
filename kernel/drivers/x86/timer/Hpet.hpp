#pragma once

#include <kernel/drivers/Timer.hpp>

#include <common/Try.hpp>

namespace driver::timer {
	struct Hpet final: driver::Timer {
		DRIVER_INSTANCE(Hpet, "hpet", "High Precision Event Timer", driver::Timer)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;
		void _on_irq(U8) override;

		using driver::Timer::schedule;
		using driver::Timer::schedule_important;
		using driver::Timer::set_timer;

		auto now() -> U32 override;
		auto now64() -> U64 override;
		auto schedule(U32 usecs, ScheduledCallback, void *data) -> U32 override;
		auto schedule_important(U32 usecs, ScheduledCallback, void *data) -> U32 override;

		auto get_timer_count() -> U8 override;
		void set_timer(U8 timer, U32 usecs, Callback, void *data) override;
		void stop_timer(U8 timer) override;
	};
}
