#include "serial.hpp"

#include "mmio.hpp"
#include "mailbox.hpp"

#include <common/types.hpp>

#include <kernel/deviceManager.hpp>
#include <kernel/driver/serial/Raspi_uart.hpp>
#include <kernel/driver/serial/Raspi_mini_uart.hpp>
#include <kernel/stdio.hpp>
#include <kernel/arch/raspi/mmio.hpp>

namespace mmio {
	using namespace arch::raspi;
}

namespace arch {
	namespace raspi {
		namespace serial {
			driver::serial::Raspi_uart      uart0((U64)mmio::Address::uart0, "Raspi UART0");
			driver::serial::Raspi_mini_uart uart1((U64)mmio::Address::uart1, "Raspi mini UART");
			#if defined(ARCH_RASPI4)
				driver::serial::Raspi_uart  uart2((U64)mmio::Address::uart2, "Raspi UART2");
				driver::serial::Raspi_uart  uart3((U64)mmio::Address::uart3, "Raspi UART3");
				driver::serial::Raspi_uart  uart4((U64)mmio::Address::uart4, "Raspi UART4");
				driver::serial::Raspi_uart  uart5((U64)mmio::Address::uart5, "Raspi UART5");
			#endif

			void init() {
				deviceManager::add_device(uart0, false);
				deviceManager::add_device(uart1, false);
				#if defined(ARCH_RASPI4)
					deviceManager::add_device(uart2, false);
					deviceManager::add_device(uart3, false);
					deviceManager::add_device(uart4, false);
					deviceManager::add_device(uart5, false);
				#endif

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
