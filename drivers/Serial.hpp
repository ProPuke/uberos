#pragma once

#include <drivers/Hardware.hpp>

#include <kernel/console.hpp>

#include <functional>

namespace driver {
	struct Serial: Hardware {
		DRIVER_TYPE(Serial, 0x81502b1, "serial", "Serial Driver", Hardware)

		virtual void set_baud(U32 set) = 0;

		virtual auto get_active_baud() -> U32 = 0;

		virtual void putc(char c) = 0;
		virtual void puts(const char *str) { while(*str) putc(*str); }
		virtual auto peekc() -> char = 0;
		virtual auto getc() -> char = 0;

		void bind_to_console() {
			if(!api.is_active()) return

			console::bind(this,
				[](void *self, char c) { return ((Serial*)self)->putc(c); },
				[](void *self) { return ((Serial*)self)->peekc(); },
				[](void *self) { return ((Serial*)self)->getc(); },
				[](void *self, const char *str) { return ((Serial*)self)->puts(str); },
				nullptr
			);
		}
	};
}
