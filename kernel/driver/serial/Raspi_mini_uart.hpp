#include <kernel/driver/Serial.hpp>

namespace driver {
	namespace serial {
		struct Raspi_mini_uart: driver::Serial {
			constexpr /**/ Raspi_mini_uart(U64 address, const char *name):
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

			void bind_stdio() override;

			U32 _specified_baud = 9600;
			
			U32 _active_baud = 9600;
		};
	}
}
