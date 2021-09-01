#pragma once

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
