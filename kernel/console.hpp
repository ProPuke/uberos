#pragma once

#include <common/types.hpp>

#undef getc
#undef putc
#undef gets
#undef puts

namespace console {
	void putc(unsigned char c);
	auto peekc() -> unsigned char;
	auto getc() -> unsigned char;
	void puts(const char *str);
	void gets(char *buffer, U32 length);

	typedef void (*PutcBinding)(void*, unsigned char c);
	typedef auto (*GetcBinding)(void*) -> unsigned char;
	typedef auto (*PeekcBinding)(void*) -> unsigned char;
	typedef void (*PutsBinding)(void*, const char *str);
	typedef void (*GetsBinding)(void*, char *buf, U32 length);

	void bind(void* binding, PutcBinding putc, PeekcBinding peekc, GetcBinding getc, PutsBinding puts = nullptr, GetsBinding gets = nullptr);
}

#include "console.inl"
