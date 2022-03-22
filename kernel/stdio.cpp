#include "stdio.hpp"

#include <common/stdlib.hpp>

namespace stdio {
	U32 indent = 0;

	namespace {
		void null_putc(unsigned char c) {}
		auto null_getc() -> unsigned char { return 0; }

		void automatic_puts(const char *str) {
			while(*str) putc(*str++);
		}

		void automatic_gets(char *buffer, U32 length) {
			char c;
			U32 i;

			for(i=0; i<length; i++){
				c=getc();

				if(c=='\r') continue;
				if(c=='\n') break;
				
				putc(c);
				buffer[i] = c;
			}

			putc('\n');
			buffer[i] = '\0';
		}
	}

	Putc putc = null_putc;
	Getc getc = null_getc;
	Puts puts = automatic_puts;
	Gets gets = automatic_gets;

	void bind(Putc putc, Getc getc, Puts puts, Gets gets) {
		stdio::putc = putc?:null_putc;
		stdio::getc = getc?:null_getc;

		stdio::puts = puts?:automatic_puts;
		stdio::gets = gets?:automatic_gets;
	}
}
