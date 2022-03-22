#include "Raspi_uart.hpp"

#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/arch/raspi/mailbox.hpp>

namespace mmio {
	using namespace arch::raspi;
}

namespace mailbox {
	using namespace arch::raspi::mailbox;
}

namespace driver {
	namespace serial {
		namespace {
			const U32 clock = 3000000;

			inline void __putc(unsigned char c) {
				if(c=='\n') __putc('\r');
				while(mmio::read_address(mmio::Address::uart0_fr) & 1<<5);
				mmio::write_address(mmio::Address::uart0_dr, c);
			}

			inline void _putc(unsigned char c) {
				mmio::PeripheralWriteGuard _guard;
				__putc(c);
			}

			inline auto __getc() -> unsigned char {
				while(mmio::read_address(mmio::Address::uart0_fr) & 1<<4);
				return mmio::read_address(mmio::Address::uart0_dr);
			}

			inline unsigned char _getc() {
				mmio::PeripheralReadGuard _guard;
				return __getc();
			}
			
			inline void __puts(const char* str) {
				while(*str) __putc(*str++);
			}

			inline void _puts(const char* str) {
				mmio::PeripheralWriteGuard _guard;
				while(*str) __putc(*str++);
			}
		}

		void Raspi_uart::set_baud(U32 set) {
			_specified_baud = set;
		}

		void Raspi_uart::enable_driver() {
			if(state==State::enabled) return;

			// Disable UART0
			mmio::write_address(mmio::Address::uart0_cr, 0x00000000);

			mailbox::PropertyMessage tags[2];
			tags[0].tag = mailbox::PropertyTag::set_clock_rate;
			tags[0].data.clock_rate.clockId = 2;
			tags[0].data.clock_rate.rate = 4000000; //4Mhz
			tags[0].data.clock_rate.skipSettingTurbo = 0;

			tags[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(tags)){
				//TODO:error in some way?
				state = State::failed;
				return;
			}

			// Setup the GPIO pin 14 && 15
			// Disable pull up/down for all GPIO pins & delay for 150 cycles
			mmio::write_address(mmio::Address::gppud, 0x00000000);
			mmio::delay(150);

			// Disable pull up/down for pin 14,15 & delay for 150 cycles
			mmio::write_address(mmio::Address::gppudclk0, 1<<14 | 1<<15);
			mmio::delay(150);

			// Write 0 to GPPUDCLK0 to make it take effect
			mmio::write_address(mmio::Address::gppudclk0, 0x00000000);

			// Clear any interrupts pending
			mmio::write_address(mmio::Address::uart0_icr, 0x7FF);

			// Set integer & fractional part of baud rate.
			// Divider = UART_CLOCK/(16 * Baud)
			// Fraction part register = (Fractional part * 64) + 0.5
			// UART_CLOCK = 3000000; Baud = 115200.

			U64 divider = 64*clock/(16*_specified_baud); //TODO:*round* the floating component to the nearest? (as in other examples) Does this matter? This rounds it down.
			mmio::write_address(mmio::Address::uart0_ibrd, divider/64);
			mmio::write_address(mmio::Address::uart0_fbrd, divider%64);

			// mmio::write_address(mmio::Address::uart0_ibrd, 2);
			// mmio::write_address(mmio::Address::uart0_fbrd, 11);

			// Enable FIFO & 8bit data transmissions (1 stop bit, no parity)
			mmio::write_address(mmio::Address::uart0_lcrh, 1<<4 | 1<<5 | 1<<6);

			// Mask all interrupts
			mmio::write_address(mmio::Address::uart0_imsc, 1<<1 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10);

			// Enable UART0, receive & transfer part of UART.
			mmio::write_address(mmio::Address::uart0_cr, 1<<0 | 1<<8 | 1<<9);

			_active_baud = _specified_baud;

			state = State::enabled;
		}
		
		void Raspi_uart::disable_driver() {
			if(state==State::disabled) return;

			mmio::write_address(mmio::Address::uart0_cr, 0x00000000);

			state = State::disabled;
		}

		void Raspi_uart::putc(unsigned char c) {
			_putc(c);
		}
		void Raspi_uart::puts(const char *str) {
			_puts(str);
		}
		auto Raspi_uart::getc() -> unsigned char {
			return _getc();
		}

		void Raspi_uart::bind_stdio() {
			if(state!=State::enabled) return;

			stdio::bind(_putc, _getc, _puts, nullptr);
		}
	}
}