#pragma once

#include <kernel/drivers/Serial.hpp>

namespace driver {
	namespace serial {
		struct Stdout final: driver::Serial {
			typedef driver::Serial Super;

			constexpr /**/ Stdout(U64 address, const char *name = "stdout"):
				Serial(address, name, "serial port")
			{}

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;

			void set_baud(U32 set) override;

			auto get_active_baud() -> U32 override;

			void putc(char c) override;
			void puts(const char *str) override;
			auto peekc() -> char override;
			auto getc() -> char override;
		};
	}
}
