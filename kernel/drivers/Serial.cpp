#include "Serial.hpp"

namespace driver {
	DriverType Serial::driverType{"serial", &Super::driverType};

	/**/ Serial::Serial(const char *name, const char *description):
		Driver(name, description)
	{
		type = &driverType;
	}

	void Serial::bind_to_console() {
		if(!api.is_active()) return

		console::bind(this,
			[](void *self, char c) { return ((Serial*)self)->putc(c); },
			[](void *self) { return ((Serial*)self)->peekc(); },
			[](void *self) { return ((Serial*)self)->getc(); },
			[](void *self, const char *str) { return ((Serial*)self)->puts(str); },
			nullptr
		);
	}
}
