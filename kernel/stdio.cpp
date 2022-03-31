#include "stdio.hpp"

#include <common/stdlib.hpp>

namespace stdio {
	U32 indent = 0;

	namespace {
		void null_putc(void*, unsigned char c) {}
		auto null_getc(void*) -> unsigned char { return 0; }
		auto null_peekc(void*) -> unsigned char { return 0; }

		void automatic_puts(void *binding, const char *str) {
			while(*str) _binding_putc(binding, *str++);
		}

		void automatic_gets(void *binding, char *buffer, U32 length) {
			char c;
			U32 i;

			for(i=0; i<length; i++){
				c = _binding_getc(binding);

				if(c=='\r') continue;
				if(c=='\n') break;
				
				_binding_putc(binding, c);
				buffer[i] = c;
			}

			_binding_putc(binding, '\n');
			buffer[i] = '\0';
		}
	}

	void *_binding = nullptr;
	Putc _binding_putc = null_putc;
	Peekc _binding_peekc = null_peekc;
	Getc _binding_getc = null_getc;
	Puts _binding_puts = automatic_puts;
	Gets _binding_gets = automatic_gets;

	void bind(void* binding, Putc putc, Peekc peekc, Getc getc, Puts puts, Gets gets) {
		_binding = binding;

		_binding_putc = putc?:null_putc;
		_binding_peekc = peekc?:null_peekc;
		_binding_getc = getc?:null_getc;

		_binding_puts = puts?:automatic_puts;
		_binding_gets = gets?:automatic_gets;
	}
}
