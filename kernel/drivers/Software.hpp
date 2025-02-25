#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	struct Software: Driver {
		DRIVER_TYPE(Software, 0x5614f63c, "software", "Software Driver", Driver)
	};
}
