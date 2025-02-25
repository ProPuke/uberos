#pragma once

#include <kernel/drivers/Serial.hpp>

#include <common/Try.hpp>

namespace driver {
	namespace serial {
		struct Raspi_uart final: driver::Serial {
			DRIVER_TYPE_CUSTOM_CTOR(Raspi_uart, 0xffcd4e57, "uart", "Raspberry Pi UART Serial", driver::Serial)

			/**/ Raspi_uart(U32 address):
				Super(DriverApi::Startup::automatic),
				_address(address)
			{ DRIVER_DECLARE_INIT(); }

			auto _on_start() -> Try<> override;
			auto _on_stop() -> Try<> override;

			void set_uart(U32 uart);
			void set_baud(U32 set) override;

			auto get_active_baud() -> U32 override { return _active_baud; }

			void putc(char c) override;
			void puts(const char *str) override;
			auto peekc() -> char override;
			auto getc() -> char override;

		private:

			U32 _address;

			U32 _specified_baud = 9600;
			U32 _active_baud = 9600;

			void _putc(char c);
			auto _peekc() -> char;
			auto _getc() -> char;
			void _puts(const char* str);
		};
	}
}
