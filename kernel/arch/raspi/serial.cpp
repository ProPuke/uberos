#include "serial.hpp"

#include <kernel/arch/raspi/mailbox.hpp>
#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/drivers.hpp>
#include <kernel/drivers/raspi/serial/Raspi_mini_uart.hpp>
#include <kernel/drivers/raspi/serial/Raspi_uart.hpp>
#include <kernel/log.hpp>

#include <common/types.hpp>

namespace mmio {
	using namespace arch::raspi;
}

namespace arch {
	namespace raspi {
		namespace serial {
			template <const char *name, const char *description>
			struct Uart: driver::serial::Raspi_uart {
				typedef driver::serial::Raspi_uart Super;
				DRIVER_TYPE(name, description)
				/**/ Uart0(U32 address): Super(address) { DRIVER_INIT(); }
			};
			template <const char *name, const char *description>
			struct UartMini: driver::serial::Raspi_mini_uart {
				typedef driver::serial::Raspi_mini_uart Super;
				DRIVER_TYPE(name, description)
				/**/ Uart0(U32 address): Super(address) { DRIVER_INIT(); }
			};

			Uart<"uart0", "Raspberry Pi UART0 Serial"> uart0(mmio::Address::uart0);
			UartMini<"uart1", "Raspberry Pi Mini UART1 Serial"> uart1(mmio::Address::uart1);
			#if defined(ARCH_RASPI4)
				Uart<"uart2", "Raspberry Pi UART2 Serial"> uart2(mmio::Address::uart2);
				Uart<"uart3", "Raspberry Pi UART3 Serial"> uart3(mmio::Address::uart3);
				Uart<"uart4", "Raspberry Pi UART4 Serial"> uart4(mmio::Address::uart4);
				Uart<"uart5", "Raspberry Pi UART5 Serial"> uart5(mmio::Address::uart5);
			#endif


			// driver::serial::Raspi_uart      uart0((U64)mmio::Address::uart0, "Raspi UART0");
			// driver::serial::Raspi_mini_uart uart1((U64)mmio::Address::uart1, "Raspi mini UART1");
			// #if defined(ARCH_RASPI4)
			// 	driver::serial::Raspi_uart  uart2((U64)mmio::Address::uart2, "Raspi UART2");
			// 	driver::serial::Raspi_uart  uart3((U64)mmio::Address::uart3, "Raspi UART3");
			// 	driver::serial::Raspi_uart  uart4((U64)mmio::Address::uart4, "Raspi UART4");
			// 	driver::serial::Raspi_uart  uart5((U64)mmio::Address::uart5, "Raspi UART5");
			// #endif

			void init() {
				drivers::install_driver(uart0, false);
				drivers::install_driver(uart1, false);
				#if defined(ARCH_RASPI4)
					drivers::install_driver(uart2, false);
					drivers::install_driver(uart3, false);
					drivers::install_driver(uart4, false);
					drivers::install_driver(uart5, false);
				#endif

				auto &serial =
					#if defined(ARCH_RASPI_UART0)
						uart0
					#elif defined(ARCH_RASPI_UART1)
						uart1
					#endif
				;

				serial.set_baud(115200);
				drivers::activate_driver(serial);

				serial.bind_to_console();

				if(serial.state==Driver::State::enabled) {
					log::Section section("arch::raspi::serial::init...");
					#if defined(ARCH_RASPI_UART0)
						log::print_info("UART0 active");
					#elif defined(ARCH_RASPI_UART1)
						log::print_info("UART1 active");
					#endif

					// char buffer[1024];
					// serial.gets(buffer, 1024);
					// log::print_info("got ", (const char *)&buffer[0]);
				}
			}
		}
	}
}
