#include "time.hpp"

#include <kernel/drivers/Timer.hpp>

namespace time {
	DriverReference<driver::Timer> timer; //TODO: make this a weak reference? It's okay if this reference gets dropped (so we don't need to keep the driver alive). We can grab it again on each use

	auto now() -> U64 {
		return 0;
		// if(!timer) timer = drivers::find_and_activate<driver::Timer>();
		// if(!timer) return 0;

		// return timer->now64();
	}
}
