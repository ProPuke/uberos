#include "time.hpp"

#include <kernel/DriverReference.hpp>

#include <drivers/Timer.hpp>

namespace time {
	AutomaticDriverReference<driver::Timer> timer;

	void init() {
		timer = drivers::find_and_activate<driver::Timer>();
	}

	auto now() -> U64 {
		if(!timer) return 0;

		return timer->now64();
	}
}
