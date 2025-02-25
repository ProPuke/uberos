#pragma once

#include <kernel/drivers/Serial.hpp>

#include <common/Try.hpp>

namespace driver {
	namespace serial {
		struct Raspi_mini_uart final: driver::Serial {
			DRIVER_TYPE_CUSTOM_CTOR(Raspi_mini_uart, 0x599cf227, "miniuart", "Raspberry Pi Mini UART Serial", driver::Serial)

			/**/ Raspi_mini_uart(U32 address):
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
