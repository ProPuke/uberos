#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	namespace processor {
		//TODO: merge into a 8259 PIC driver?

		struct Apic: Driver {
			typedef Driver Super;

			static DriverType driverType;

			/**/ Apic();
			/**/ Apic(U64 address);

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;
		};
	}
}
