#pragma once

#include <kernel/driver/Serial.hpp>

namespace driver {
	namespace serial {
		struct Raspi_uart final: driver::Serial {
			constexpr /**/ Raspi_uart(U64 address, const char *name = "Raspi UART"):
				Serial(address, name, "serial port")
			{
				is_builtin = true;
			}

			void set_uart(U32 uart);
			void set_baud(U32 set) override;

			void enable_driver() override;
			void disable_driver() override;

			auto get_active_baud() -> U32 override { return _active_baud; }

			void putc(unsigned char c) override;
			void puts(const char *str) override;
			auto getc() -> unsigned char override;

		private:

			U32 _specified_baud = 9600;
			
			U32 _active_baud = 9600;

			void __putc(unsigned char c);
			void _putc(unsigned char c);
			auto __getc() -> unsigned char;
			auto _getc() -> unsigned char;
			void __puts(const char* str);
			void _puts(const char* str);
		};
	}
}
