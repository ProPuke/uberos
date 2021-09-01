#pragma once

#include "serial.hpp"

#include "mmio.hpp"

namespace arch {
	namespace raspi {
		namespace serial {
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
		}
	}
}