#include "serial.hpp"

#include "mmio.hpp"
#include "mailbox.hpp"

#include <common/types.hpp>

#include <kernel/deviceManager.hpp>
#include <kernel/driver/serial/Raspi_uart.hpp>
#include <kernel/driver/serial/Raspi_mini_uart.hpp>
#include <kernel/stdio.hpp>
#include <kernel/arch/raspi/mmio.hpp>

namespace arch {
	namespace raspi {
		namespace serial {
			driver::serial::Raspi_uart uart0((U64)mmio::arch::raspi::Address::uart0_base, "Raspi UART0");
			driver::serial::Raspi_mini_uart uart1((U64)mmio::arch::raspi::Address::uart1_base, "Raspi mini UART");

			void init() {
				deviceManager::add_device(uart0, false);
				deviceManager::add_device(uart1, false);

				auto &serial =
					#if defined(ARCH_RASPI_UART0)
						uart0
					#elif defined(ARCH_RASPI_UART1)
						uart1
					#endif
				;

				serial.set_baud(115200);
				serial.enable_driver();
				
				serial.bind_stdio();

				// if(serial.state==Driver::State::enabled){
				// 	stdio::bind(
				// 		[](unsigned char c) { return serial.putc(c); },
				// 		[]() { return serial.getc(); },
				// 		[](const char *s) { return serial.puts(s); },
				// 		[](char *buffer, U32 length) { return serial.gets(buffer, length); }
				// 	);
				// }

				if(serial.state==Driver::State::enabled) {
					stdio::Section section("arch::raspi::serial::init...");
					#if defined(ARCH_RASPI_UART0)
						stdio::print_info("UART0 active");
					#elif defined(ARCH_RASPI_UART1)
						stdio::print_info("UART1 active");
					#endif
				}
			}
		}
	}
}
