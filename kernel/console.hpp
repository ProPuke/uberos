#pragma once

#include <common/types.hpp>

#undef getc
#undef putc
#undef gets
#undef puts

namespace console {
	void putc(char c);
	auto peekc() -> char;
	auto getc() -> char;
	void puts(const char *str);
	void gets(char *buffer, U32 length);

	typedef void (*PutcBinding)(void*, char c);
	typedef auto (*GetcBinding)(void*) -> char;
	typedef auto (*PeekcBinding)(void*) -> char;
	typedef void (*PutsBinding)(void*, const char *str);
	typedef void (*GetsBinding)(void*, char *buf, U32 length);

	void bind(void* binding, PutcBinding putc, PeekcBinding peekc, GetcBinding getc, PutsBinding puts = nullptr, GetsBinding gets = nullptr);
}

#include "console.inl"
