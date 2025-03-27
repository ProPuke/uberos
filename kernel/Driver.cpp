#include "Driver.hpp"

#include <kernel/drivers.hpp>

/**/ Driver::Driver(DriverApi::Startup startup):
	api(startup)
{
	DRIVER_DECLARE_INIT();

	(void)drivers::install_driver(*this);
}
