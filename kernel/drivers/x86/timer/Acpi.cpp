#include "Acpi.hpp"

namespace driver {
	namespace timer {
		DriverType Acpi::driverType{"acpi", &Acpi::Super::driverType};

		/**/ Acpi::Acpi(const char *name, const char *description):
			Driver(name, description)
		{
			type = &driverType;
		}

		auto Acpi::_on_start() -> bool {
			return true;
		}

		auto Acpi::_on_stop() -> bool {
			return true;
		}
	}
}