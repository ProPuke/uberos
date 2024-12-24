#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	namespace processor {
		struct Acpi: Driver {
			/**/ Acpi();
			/**/ Acpi(U64 address);
		};
	}
}
