#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	struct Hardware: Driver {
		DRIVER_TYPE(Hardware, 0x68216bb2, "hardware", "Hardware Driver", Driver)
	};
}
