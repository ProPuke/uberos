#include "Processor.hpp"

namespace driver {
	DriverType Processor::driverType{"processor", &Super::driverType};

	/**/ Processor::Processor(const char *name, const char *processor_arch, const char *description):
		Driver(name, description),
		processor_arch(processor_arch)
	{
		type = &driverType;
	}
}
