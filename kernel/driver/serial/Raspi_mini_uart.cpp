#include "Raspi_mini_uart.hpp"

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
			const U32 uart_clock = 500000000;

			U32 baud_to_uart_reg(U32 baud) {
				return uart_clock/(baud*8)-1;
			}

			inline void __putc(unsigned char c) {
				if(c=='\n') __putc('\r');

				while(!(mmio::read_address(mmio::Address::uart1_mu_lsr) & 1<<5));
				mmio::write_address(mmio::Address::uart1_mu_io, c);
			}

			inline void _putc(unsigned char c) {
				mmio::PeripheralWriteGuard _guard;
				__putc(c);
			}

			inline unsigned char __getc() {
				while(!(mmio::read_address(mmio::Address::uart1_mu_lsr) & 1<<1));
				return mmio::read_address(mmio::Address::uart1_mu_io);
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

		void Raspi_mini_uart::set_baud(U32 set) {
			_specified_baud = set;
		}

		void Raspi_mini_uart::enable_driver() {
			if(state==State::enabled) return;

			{
				// initialize UART1
				mmio::write_address(mmio::Address::uart1_enable, mmio::read_address(mmio::Address::uart1_enable)|1); // enable UART1, AUX mini uart
				mmio::write_address(mmio::Address::uart1_mu_cntl, 0);
				mmio::write_address(mmio::Address::uart1_mu_lcr, 3); // 8 bits
				mmio::write_address(mmio::Address::uart1_mu_mcr, 0);
				mmio::write_address(mmio::Address::uart1_mu_ier, 0);
				mmio::write_address(mmio::Address::uart1_mu_iir, 0xc6); // disable interrupts
				mmio::write_address(mmio::Address::uart1_mu_baud, baud_to_uart_reg(_specified_baud));


				// map UART1 to GPIO pins
				U32 r = mmio::read_address(mmio::Address::gpfsel1);
				r &= ~((7<<12)|(7<<15)); // gpio14, gpio15
				r |= (2<<12)|(2<<15); // alt5
				mmio::write_address(mmio::Address::gpfsel1, r);
				mmio::write_address(mmio::Address::gppud, 0); // enable pins 14 and 15
				mmio::delay(150);
				mmio::write_address(mmio::Address::gppudclk0, (1<<14)|(1<<15));
				mmio::delay(150);
				mmio::write_address(mmio::Address::gppudclk0, 0); // flush GPIO setup
				mmio::write_address(mmio::Address::uart1_mu_cntl, 3); // enable Tx, Rx
			}

			_active_baud = _specified_baud;

			state = State::enabled;
		}
		
		void Raspi_mini_uart::disable_driver() {
			if(state==State::disabled) return;

			mmio::write_address(mmio::Address::uart1_enable, mmio::read_address(mmio::Address::uart1_enable)&~1);

			state = State::disabled;
		}

		void Raspi_mini_uart::putc(unsigned char c) {
			_putc(c);
		}
		void Raspi_mini_uart::puts(const char *str) {
			_puts(str);
		}
		auto Raspi_mini_uart::getc() -> unsigned char {
			return _getc();
		}

		void Raspi_mini_uart::bind_stdio() {
			if(state!=State::enabled) return;

			stdio::bind(_putc, _getc, _puts, nullptr);
		}
	}
}
