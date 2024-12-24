#include "Timer.hpp"

namespace driver {
	DriverType Timer::driverType{"timer", &Super::driverType};

	/**/ Timer::Timer(const char *name, const char *description):
		Driver(name, description)
	{
		type = &driverType;
	}
}
