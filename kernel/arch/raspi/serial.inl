#pragma once

#include "serial.hpp"

#include "mmio.hpp"

namespace mmio {
	using namespace arch::raspi;
}

namespace arch {
	namespace raspi {
		namespace serial {
			#ifdef ARCH_RASPI_UART0
				inline void putc(unsigned char c) {
					if(c=='\n') putc('\r');
					while(mmio::read(mmio::Address::uart0_fr) & 1<<5);
					mmio::write(mmio::Address::uart0_dr, c);
				}

				inline unsigned char getc() {
					while(mmio::read(mmio::Address::uart0_fr) & 1<<4);
					return mmio::read(mmio::Address::uart0_dr);
				}

				inline void puts(const char* str) {
					while(*str) putc(*str++);
				}
			#endif

			#ifdef ARCH_RASPI_UART1
				inline void _putc(unsigned char c) {
					if(c=='\n') _putc('\r');

					while(!(mmio::read(mmio::Address::uart1_mu_lsr) & 1<<5));
					mmio::write(mmio::Address::uart1_mu_io, c);
				}

				inline void putc(unsigned char c) {
					mmio::PeripheralWriteGuard _guard;
					_putc(c);
				}

				inline unsigned char _getc() {
					while(!(mmio::read(mmio::Address::uart1_mu_lsr) & 1<<1));
					return mmio::read(mmio::Address::uart1_mu_io);
				}

				inline unsigned char getc() {
					mmio::PeripheralReadGuard _guard;
					return _getc();
				}

				inline void _puts(const char* str) {
					while(*str) _putc(*str++);
				}

				inline void puts(const char* str) {
					mmio::PeripheralWriteGuard _guard;
					while(*str) _putc(*str++);
				}
			#endif
		}
	}
}