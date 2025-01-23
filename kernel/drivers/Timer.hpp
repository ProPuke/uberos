#pragma once

#include <kernel/drivers/Hardware.hpp>

namespace driver {
	struct Timer: Hardware {
		DRIVER_TYPE(Timer, "timer", "Hardware Timer Driver", Hardware)

		virtual auto now() -> U32 = 0;
		virtual auto now64() -> U64 = 0;
		virtual void wait(U32 usecs) = 0;
	};
}
