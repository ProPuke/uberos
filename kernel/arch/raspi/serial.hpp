#pragma once

#if !defined(ARCH_RASPI_UART0) && !defined(ARCH_RASPI_UART1)
	#define ARCH_RASPI_UART1
#endif

namespace arch {
	namespace raspi {
		namespace serial {
			void init();

			void putc(unsigned char c);
			unsigned char getc();
			void puts(const char *str);
		}
	}
}

#include "serial.inl"
