#pragma once

#include <drivers/Clock.hpp>

namespace driver::clock {
	struct CmosRtc: Clock {
		DRIVER_INSTANCE(CmosRtc, 0x80110f10, "cmosRtc", "CMOS Real-Time Clock", Clock)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto get_date() -> Date override;
		auto get_time() -> Time override;
	};
}
