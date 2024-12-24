#pragma once

#include <kernel/console.hpp>
#include <kernel/Driver.hpp>

#include <functional>

namespace driver {
	struct Serial: Driver {
		typedef Driver Super;

		static DriverType driverType;

		/**/ Serial(const char *name, const char *description);

		virtual void set_baud(U32 set) = 0;

		virtual auto get_active_baud() -> U32 = 0;

		virtual void putc(char c) = 0;
		virtual void puts(const char *str) { while(*str) putc(*str); }
		virtual auto peekc() -> char = 0;
		virtual auto getc() -> char = 0;

		void bind_to_console();
	};
}
