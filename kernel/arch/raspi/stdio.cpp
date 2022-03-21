#include "stdio.hpp"

#include "serial.hpp"

#include <common/stdlib.hpp>

namespace serial {
	using namespace arch::raspi::serial;
}

namespace stdio {
	U32 indent = 0;

	char getc() {
		return serial::getc();
	}

	void _putc(char c) {
		serial::putc(c);
	}

	void _puts(const char *str) {
		serial::puts(str);
		// while(*str) putc(*str++);
	}

	void gets(char *buf, int buflen) {
		char c;
		int i;

		for (i=0; (c=getc())!='\r' && buflen>1; i++, buflen--){
			_putc(c);
			buf[i] = c;
		}

		_putc('\n');

		if (c == '\n') {
			buf[i] = '\0';
		} else {
			buf[buflen-1] = '\0';
		}
	}
}
