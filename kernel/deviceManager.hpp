#pragma once

#include <kernel/Driver.hpp>

namespace deviceManager {
	extern LList<Driver> devices;

	void add_device(Driver &device, bool enable = true);
}
