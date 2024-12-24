#include "Driver.hpp"

DriverType Driver::driverType{"driver", nullptr};

/**/ Driver::Driver(const char *name, const char *description):
	name(name),
	type(&driverType),
	description(description)
{}

/**/ Driver::~Driver() {}
