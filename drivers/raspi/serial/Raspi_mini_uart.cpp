#include "Raspi_mini_uart.hpp"

#include <kernel/arch/raspi/mailbox.hpp>
#include <kernel/arch/raspi/mmio.hpp>

namespace mmio {
	using namespace arch::raspi::mmio;
}

//TODO:the gpio mmio addresses still needing abstracting away, in here. Those should prob be passed to the driver

namespace driver {
	namespace serial {
		namespace {
			const U32 uart_clock = 500000000;

			enum struct Address:U32 {
				irq        = 0x00, // bits 0, 1 and 2 are 1 if there is a pending mini uart interrupt, sp1 interrupt and sp2 itnerrupt
				enable     = 0x04,
				mu_io      = 0x40, // i/o data
				mu_ier     = 0x44, // interrupt enable
				mu_iir     = 0x48, // interrupt identity
				mu_lcr     = 0x4C, // line control
				mu_mcr     = 0x50, // modem control
				mu_lsr     = 0x54, // line status
				mu_msr     = 0x58, // model status
				mu_scratch = 0x5C, // scratch
				mu_cntl    = 0x60, // extra control
				mu_stat    = 0x64, // extra status
				mu_baud    = 0x68, // baudrate
			};

			U32 baud_to_uart_reg(U32 baud) {
				return uart_clock/(baud*8)-1;
			}
		}

		void Raspi_mini_uart::set_baud(U32 set) {
			_specified_baud = set;
		}

		auto Raspi_mini_uart::_on_start() -> Try<> {
			{
				// initialize UART1
				mmio::write32(_address+(U32)Address::enable, mmio::read32(_address+(U32)Address::enable)|1); // enable UART1, AUX mini uart
				mmio::write32(_address+(U32)Address::mu_cntl, 0);
				mmio::write32(_address+(U32)Address::mu_lcr, 3); // 8 bits
				mmio::write32(_address+(U32)Address::mu_mcr, 0);
				mmio::write32(_address+(U32)Address::mu_ier, 0);
				mmio::write32(_address+(U32)Address::mu_iir, 0xc6); // disable interrupts
				mmio::write32(_address+(U32)Address::mu_baud, baud_to_uart_reg(_specified_baud));


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
				mmio::write32(_address+(U32)Address::mu_cntl, 3); // enable Tx, Rx
			}

			_active_baud = _specified_baud;

			return {};
		}
		
		auto Raspi_mini_uart::_on_stop() -> Try<> {
			mmio::write32(_address+(U32)Address::enable, mmio::read32(_address+(U32)Address::enable)&~1);

			return {};
		}

		void Raspi_mini_uart::putc(char c) {
			mmio::PeripheralWriteGuard _guard;
			_putc(c);
		}
		void Raspi_mini_uart::puts(const char *str) {
			mmio::PeripheralWriteGuard _guard;
			while(*str) _putc(*str++);
		}
		auto Raspi_mini_uart::peekc() -> char {
			mmio::PeripheralReadGuard _guard;
			return _peekc();
		}
		auto Raspi_mini_uart::getc() -> char {
			mmio::PeripheralReadGuard _guard;
			return _getc();
		}

		void Raspi_mini_uart::_putc(char c) {
			if(c=='\n') _putc('\r');

			while(!(mmio::read32(_address+(U32)Address::mu_lsr) & 1<<5));
			mmio::write32(_address+(U32)Address::mu_io, c);
		}

		auto Raspi_mini_uart::_peekc() -> char {
			if(mmio::read32(_address+(U32)Address::mu_lsr) & 1<<0){
				return mmio::read32(_address+(U32)Address::mu_io);
			}else{
				return 0;
			}
		}

		auto Raspi_mini_uart::_getc() -> char {
			while(!(mmio::read32(_address+(U32)Address::mu_lsr) & 1<<0));
			return mmio::read32(_address+(U32)Address::mu_io);
		}

		void Raspi_mini_uart::_puts(const char* str) {
			while(*str) _putc(*str++);
		}
	}
}
