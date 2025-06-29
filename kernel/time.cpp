#include "time.hpp"

#include <kernel/DriverReference.hpp>

#include <drivers/Timer.hpp>

namespace time {
	constinit AutomaticDriverReference<driver::Timer> timer;

	void init() {
		timer.get();
	}

	auto now() -> U64 {
		if(!timer) return 0;

		return timer->now64();
	}
}
