#include "Raspi_uart.hpp"

#include <kernel/arch/raspi/mailbox.hpp>
#include <kernel/arch/raspi/mmio.hpp>

namespace mailbox {
	using namespace arch::raspi::mailbox;
}

namespace mmio {
	using namespace arch::raspi::mmio;
}

namespace driver {
	namespace serial {
		namespace {
			//FIXME: do something about the shared state between multiple of these. They manipulate some of the same gpio registers
			
			const U32 clock = 3000000;

			enum struct Address:U32 {
				dr      = 0x00,
				rsrecr  = 0x04,
				fr      = 0x18,
				ilpr    = 0x20,
				ibrd    = 0x24,
				fbrd    = 0x28,
				lcrh    = 0x2C,
				cr      = 0x30,
				ifls    = 0x34,
				imsc    = 0x38,
				ris     = 0x3C,
				mis     = 0x40,
				icr     = 0x44,
				dmacr   = 0x48,
				itcr    = 0x80,
				itip    = 0x84,
				itop    = 0x88,
				tdr     = 0x8C,
			};
		}

		auto Raspi_uart::_on_start() -> Try<> {
			// Disable UART0
			mmio::write32(address+(U32)Address::cr, 0x00000000);

			mailbox::PropertyMessage tags[2];
			tags[0].tag = mailbox::PropertyTag::set_clock;
			tags[0].data.setClock.clock = mailbox::PropertyMessage::Data::Clock::uart;
			tags[0].data.setClock.rate = 4000000; //4Mhz
			tags[0].data.setClock.skipSettingTurbo = 0;

			tags[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(tags)){
				//TODO:error in some way?
				state = State::failed;
				return {"Error sending to mailbox"};
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
			mmio::write32(address+(U32)Address::icr, 0x7FF);

			// Set integer & fractional part of baud rate.
			U64 divider = 64*clock/(16*_specified_baud); //TODO:*round* the floating component to the nearest? (as in other examples) Does this matter? This rounds it down.
			mmio::write32(address+(U32)Address::ibrd, divider/64);
			mmio::write32(address+(U32)Address::fbrd, divider%64);

			// mmio::write32(address+(U32)Address::ibrd, 2);
			// mmio::write32(address+(U32)Address::fbrd, 11);

			// Enable FIFO & 8bit data transmissions (1 stop bit, no parity)
			mmio::write32(address+(U32)Address::lcrh, 1<<4 | 1<<5 | 1<<6);

			// Mask all interrupts
			mmio::write32(address+(U32)Address::imsc, 1<<1 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10);

			// Enable UART0, receive & transfer part of UART.
			mmio::write32(address+(U32)Address::cr, 1<<0 | 1<<8 | 1<<9);

			_active_baud = _specified_baud;

			return {};
		}
		
		auto Raspi_uart::_on_stop() -> Try<> {
			mmio::write32(address+(U32)Address::cr, 0x00000000);

			return {};
		}

		void Raspi_uart::set_baud(U32 set) {
			_specified_baud = set;
		}

		void Raspi_uart::putc(char c) {
			mmio::PeripheralWriteGuard _guard;
			_putc(c);
		}
		void Raspi_uart::puts(const char *str) {
			mmio::PeripheralWriteGuard _guard;
			while(*str) _putc(*str++);
		}
		auto Raspi_uart::peekc() -> char {
			mmio::PeripheralReadGuard _guard;
			return _peekc();
		}
		auto Raspi_uart::getc() -> char {
			mmio::PeripheralReadGuard _guard;
			return _getc();
		}

		void Raspi_uart::_putc(char c) {
			if(c=='\n') _putc('\r');
			while(mmio::read32(address+(U32)Address::fr) & 1<<5);
			mmio::write32(address+(U32)Address::dr, c);
		}

		auto Raspi_uart::_peekc() -> char {
			if(!(mmio::read32(address+(U32)Address::fr) & 1<<4)){
				return mmio::read32(address+(U32)Address::dr);
			}else{
				return 0;
			}
		}
		
		auto Raspi_uart::_getc() -> char {
			while(mmio::read32(address+(U32)Address::fr) & 1<<4);
			return mmio::read32(address+(U32)Address::dr);
		}
		
		void Raspi_uart::_puts(const char* str) {
			while(*str) _putc(*str++);
		}
	}
}