#pragma once

#include "console.hpp"

namespace console {
	extern void* _binding;
	extern PutcBinding _binding_putc;
	extern GetcBinding _binding_peekc;
	extern GetcBinding _binding_getc;
	extern PutsBinding _binding_puts;
	extern GetsBinding _binding_gets;

	inline auto putc(char c) -> void {
		return _binding_putc(_binding, c);
	}
	inline auto peekc() -> char {
		return _binding_peekc(_binding);
	}
	inline auto getc() -> char {
		return _binding_getc(_binding);
	}
	inline auto puts(const char *str) -> void {
		return _binding_puts(_binding, str);
	}
	inline void gets(char *buffer, U32 length) {
		return _binding_gets(_binding, buffer, length);
	}
}
