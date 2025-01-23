#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	struct Software: Driver {
		DRIVER_TYPE(Software, "software", "Software Driver", Driver)
	};
}
