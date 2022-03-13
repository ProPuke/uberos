#include "serial.hpp"

#include "mmio.hpp"
#include "mailbox.hpp"
#include <common/types.hpp>
#include <kernel/stdio.hpp>

namespace arch {
	namespace raspi {
		namespace serial {
			const U32 aux_uart_clock = 500000000;

			U32 baud_to_reg(U32 baud) {
				return aux_uart_clock/(baud*8)-1;
			}

			void init() {
				#ifdef ARCH_RASPI_UART0
					{ //uart 0
						// Disable UART0
						mmio::write(mmio::Address::uart0_cr, 0x00000000);

						mailbox::PropertyMessage tags[2];
						tags[0].tag = mailbox::PropertyTag::set_clock_rate;
						tags[0].data.clock_rate.clockId = 2;
						tags[0].data.clock_rate.rate = 4000000; //4Mhz
						tags[0].data.clock_rate.skipSettingTurbo = 0;

						tags[1].tag = mailbox::PropertyTag::null_tag;

						if(!mailbox::send_messages(tags)){
							//TODO:error in some way?
						}

						// Setup the GPIO pin 14 && 15
						// Disable pull up/down for all GPIO pins & delay for 150 cycles
						mmio::write(mmio::Address::gppud, 0x00000000);
						mmio::delay(150);

						// Disable pull up/down for pin 14,15 & delay for 150 cycles
						mmio::write(mmio::Address::gppudclk0, 1<<14 | 1<<15);
						mmio::delay(150);

						// Write 0 to GPPUDCLK0 to make it take effect
						mmio::write(mmio::Address::gppudclk0, 0x00000000);

						// Clear any interrupts pending
						mmio::write(mmio::Address::uart0_icr, 0x7FF);

						// Set integer & fractional part of baud rate.
						// Divider = UART_CLOCK/(16 * Baud)
						// Fraction part register = (Fractional part * 64) + 0.5
						// UART_CLOCK = 3000000; Baud = 115200.

						// // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
						mmio::write(mmio::Address::uart0_ibrd, 1);
						// // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
						mmio::write(mmio::Address::uart0_fbrd, 40);

						// mmio::write(mmio::Address::uart0_ibrd, 2);
						// mmio::write(mmio::Address::uart0_fbrd, 11);

						// Enable FIFO & 8bit data transmissions (1 stop bit, no parity)
						mmio::write(mmio::Address::uart0_lcrh, 1<<4 | 1<<5 | 1<<6);

						// Mask all interrupts
						mmio::write(mmio::Address::uart0_imsc, 1<<1 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10);

						// Enable UART0, receive & transfer part of UART.
						mmio::write(mmio::Address::uart0_cr, 1<<0 | 1<<8 | 1<<9);

						{
							stdio::Section section("arch::raspi::serial::init...");
							stdio::print_info("UART0 active");
						}
					}
				#endif

				#ifdef ARCH_RASPI_UART1
					{ //uart 1
						// initialize UART
						mmio::write(mmio::Address::uart1_enable, mmio::read(mmio::Address::uart1_enable)|1); // enable UART1, AUX mini uart
						mmio::write(mmio::Address::uart1_mu_cntl, 0);
						mmio::write(mmio::Address::uart1_mu_lcr, 3); // 8 bits
						mmio::write(mmio::Address::uart1_mu_mcr, 0);
						mmio::write(mmio::Address::uart1_mu_ier, 0);
						mmio::write(mmio::Address::uart1_mu_iir, 0xc6); // disable interrupts
						mmio::write(mmio::Address::uart1_mu_baud, baud_to_reg(115200));


						// map UART1 to GPIO pins
						U32 r = mmio::read(mmio::Address::gpfsel1);
						r &= ~((7<<12)|(7<<15)); // gpio14, gpio15
						r |= (2<<12)|(2<<15); // alt5
						mmio::write(mmio::Address::gpfsel1, r);
						mmio::write(mmio::Address::gppud, 0); // enable pins 14 and 15
						mmio::delay(150);
						mmio::write(mmio::Address::gppudclk0, (1<<14)|(1<<15));
						mmio::delay(150);
						mmio::write(mmio::Address::gppudclk0, 0); // flush GPIO setup
						mmio::write(mmio::Address::uart1_mu_cntl, 3); // enable Tx, Rx

						{
							stdio::Section section("arch::raspi::serial::init...");
							stdio::print_info("UART1 active");
						}
					}
				#endif
			}
		}
	}
}
