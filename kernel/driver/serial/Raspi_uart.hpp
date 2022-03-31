#pragma once

#include <kernel/driver/Serial.hpp>

namespace driver {
	namespace serial {
		struct Raspi_uart final: driver::Serial {
			constexpr /**/ Raspi_uart(U64 address, const char *name = "Raspi UART"):
				Serial(address, name, "serial port")
			{}

			void set_uart(U32 uart);
			void set_baud(U32 set) override;

			void _on_driver_enable() override;
			void _on_driver_disable() override;

			auto get_active_baud() -> U32 override { return _active_baud; }

			void putc(unsigned char c) override;
			void puts(const char *str) override;
			auto peekc() -> unsigned char override;
			auto getc() -> unsigned char override;

		private:

			U32 _specified_baud = 9600;
			
			U32 _active_baud = 9600;

			void _putc(unsigned char c);
			auto _peekc() -> unsigned char;
			auto _getc() -> unsigned char;
			void _puts(const char* str);
		};
	}
}
