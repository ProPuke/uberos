#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	struct Hardware: Driver {
		DRIVER_TYPE(Hardware, "hardware", "Hardware Driver", Driver)
	};
}
