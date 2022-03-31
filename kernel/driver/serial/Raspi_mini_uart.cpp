#include "Raspi_mini_uart.hpp"

#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/arch/raspi/mailbox.hpp>

namespace mmio {
	using namespace arch::raspi;
}

namespace mailbox {
	using namespace arch::raspi::mailbox;
}

//TODO:the gpio mmio addresses still needing abstracting away, in here. Those should prob be passed to the driver

namespace driver {
	namespace serial {
		namespace {
			const U32 uart_clock = 500000000;

			enum struct Address:U32 {
				enable     = 0x04,
				mu_io      = 0x40,
				mu_ier     = 0x44,
				mu_iir     = 0x48,
				mu_lcr     = 0x4C,
				mu_mcr     = 0x50,
				mu_lsr     = 0x54,
				mu_msr     = 0x58,
				mu_scratch = 0x5C,
				mu_cntl    = 0x60,
				mu_stat    = 0x64,
				mu_baud    = 0x68,
			};

			U32 baud_to_uart_reg(U32 baud) {
				return uart_clock/(baud*8)-1;
			}
		}

		void Raspi_mini_uart::set_baud(U32 set) {
			_specified_baud = set;
		}

		void Raspi_mini_uart::_on_driver_enable() {
			if(state==State::enabled) return;

			{
				// initialize UART1
				mmio::write32(address+(U32)Address::enable, mmio::read32(address+(U32)Address::enable)|1); // enable UART1, AUX mini uart
				mmio::write32(address+(U32)Address::mu_cntl, 0);
				mmio::write32(address+(U32)Address::mu_lcr, 3); // 8 bits
				mmio::write32(address+(U32)Address::mu_mcr, 0);
				mmio::write32(address+(U32)Address::mu_ier, 0);
				mmio::write32(address+(U32)Address::mu_iir, 0xc6); // disable interrupts
				mmio::write32(address+(U32)Address::mu_baud, baud_to_uart_reg(_specified_baud));


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
				mmio::write32(address+(U32)Address::mu_cntl, 3); // enable Tx, Rx
			}

			_active_baud = _specified_baud;

			state = State::enabled;
		}
		
		void Raspi_mini_uart::_on_driver_disable() {
			if(state==State::disabled) return;

			mmio::write32(address+(U32)Address::enable, mmio::read32(address+(U32)Address::enable)&~1);

			state = State::disabled;
		}

		void Raspi_mini_uart::putc(unsigned char c) {
			mmio::PeripheralWriteGuard _guard;
			_putc(c);
		}
		void Raspi_mini_uart::puts(const char *str) {
			mmio::PeripheralWriteGuard _guard;
			while(*str) _putc(*str++);
		}
		auto Raspi_mini_uart::peekc() -> unsigned char {
			mmio::PeripheralReadGuard _guard;
			return _peekc();
		}
		auto Raspi_mini_uart::getc() -> unsigned char {
			mmio::PeripheralReadGuard _guard;
			return _getc();
		}

		void Raspi_mini_uart::_putc(unsigned char c) {
			if(c=='\n') _putc('\r');

			while(!(mmio::read32(address+(U32)Address::mu_lsr) & 1<<5));
			mmio::write32(address+(U32)Address::mu_io, c);
		}

		auto Raspi_mini_uart::_peekc() -> unsigned char {
			if(mmio::read32(address+(U32)Address::mu_lsr) & 1<<1){
				return mmio::read32(address+(U32)Address::mu_io);
			}else{
				return 0;
			}
		}

		auto Raspi_mini_uart::_getc() -> unsigned char {
			while(!(mmio::read32(address+(U32)Address::mu_lsr) & 1<<1));
			return mmio::read32(address+(U32)Address::mu_io);
		}

		void Raspi_mini_uart::_puts(const char* str) {
			while(*str) _putc(*str++);
		}
	}
}
