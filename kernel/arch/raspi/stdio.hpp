#pragma once

#include <common/stdlib.hpp>

namespace stdio {
	char getc();
	void putc(char c);
	void puts(const char *str);
	void gets(char *buf, int buflen);
}
