#pragma once

#include <kernel/drivers/Timer.hpp>

namespace driver {
	namespace timer {
		struct Acpi final: Driver {
			typedef Driver Super;

			static DriverType driverType;

			/**/ Acpi(const char *name = "ACPI", const char *description = "Advanced Configuration and Power Interface");

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;

		protected:
		};
	}
}
