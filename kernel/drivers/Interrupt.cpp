#include "Interrupt.hpp"

namespace driver {
	DriverType Interrupt::driverType{"interrupt", &Super::driverType};

	/**/ Interrupt::Interrupt(const char *name, const char *description):
		Driver(name, description)
	{
		type = &driverType;
	}
}
