#pragma once

#include <kernel/drivers/Serial.hpp>

namespace driver {
	namespace serial {
		struct Raspi_mini_uart final: driver::Serial {
			typedef driver::Serial Super;

			/**/ Raspi_mini_uart(U64 address, const char *name):
				Serial(address, name, "serial port")
			{}

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;

			void set_uart(U32 uart);
			void set_baud(U32 set) override;

			auto get_active_baud() -> U32 override { return _active_baud; }

			void putc(char c) override;
			void puts(const char *str) override;
			auto peekc() -> char override;
			auto getc() -> char override;

		private:

			U32 _specified_baud = 9600;
			
			U32 _active_baud = 9600;

			void _putc(char c);
			auto _peekc() -> char;
			auto _getc() -> char;
			void _puts(const char* str);
		};
	}
}
