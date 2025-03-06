#pragma once

#include <drivers/Hardware.hpp>

namespace driver {
	struct Clock: Hardware {
		DRIVER_TYPE(Clock, 0x637db4b6, "clock", "Clock Driver", Hardware)

		struct Date {
			U16 year;
			U8 month; // 0..11
			U8 date; // 0..30
		};

		struct Time {
			U8 hours; // 0..23
			U8 minutes; // 0..59
			U8 seconds; // 0..59
		};	

		virtual auto get_date() -> Date = 0;
		virtual auto get_time() -> Time = 0;
	};
}
