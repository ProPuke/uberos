#pragma once

#include <kernel/drivers/Timer.hpp>

#include <common/Try.hpp>

namespace driver::timer {
	struct Hpet final: driver::Timer {
		DRIVER_INSTANCE(Hpet, "hpet", "High Precision Event Timer", driver::Timer)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto now() -> U32 override;
		auto now64() -> U64 override;
		void wait(U32 usecs) override;
	};
}
