#include "serial.hpp"

#include "mmio.hpp"
#include "mailbox.hpp"
#include "../../test/uart.hpp"
#include <common/types.hpp>

namespace arch {
	namespace raspi {
		namespace serial {
			void init() {
				uart_init();
				
				// // Disable UART0
				// mmio::write(mmio::Address::uart0_cr, 0x00000000);

				// mailbox::PropertyMessage tags[2];
				// tags[0].tag = mailbox::PropertyTag::set_clock_rate;
				// tags[0].data.clock_rate.clockId = 2;
				// tags[0].data.clock_rate.rate = 4000000; //4Mhz
				// tags[0].data.clock_rate.skipSettingTurbo = 0;

				// tags[1].tag = mailbox::PropertyTag::null_tag;

				// if(!mailbox::send_messages(tags)){
				// 	//TODO:error in some way?
				// }

				// // Setup the GPIO pin 14 && 15
				// // Disable pull up/down for all GPIO pins & delay for 150 cycles
				// mmio::write(mmio::Address::gppud, 0x00000000);
				// mmio::delay(150);

				// // Disable pull up/down for pin 14,15 & delay for 150 cycles
				// mmio::write(mmio::Address::gppudclk0, 1<<14 | 1<<15);
				// mmio::delay(150);

				// // Write 0 to GPPUDCLK0 to make it take effect
				// mmio::write(mmio::Address::gppudclk0, 0x00000000);

				// // Clear any interrupts pending
				// mmio::write(mmio::Address::uart0_icr, 0x7FF);

				// // Set integer & fractional part of baud rate.
				// // Divider = UART_CLOCK/(16 * Baud)
				// // Fraction part register = (Fractional part * 64) + 0.5
				// // UART_CLOCK = 3000000; Baud = 115200.

				// // // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
				// mmio::write(mmio::Address::uart0_ibrd, 1);
				// // // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
				// mmio::write(mmio::Address::uart0_fbrd, 40);

				// // mmio::write(mmio::Address::uart0_ibrd, 2);
				// // mmio::write(mmio::Address::uart0_fbrd, 11);

				// // Enable FIFO & 8bit data transmissions (1 stop bit, no parity)
				// mmio::write(mmio::Address::uart0_lcrh, 1<<4 | 1<<5 | 1<<6);

				// // Mask all interrupts
				// mmio::write(mmio::Address::uart0_imsc, 1<<1 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10);

				// // Enable UART0, receive & transfer part of UART.
				// mmio::write(mmio::Address::uart0_cr, 1<<0 | 1<<8 | 1<<9);

				puts("arch::raspi::serial::init... OK\n\n");
			}
		}
	}
}
