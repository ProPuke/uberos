#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	struct Serial: Driver {
		constexpr /**/ Serial(U64 address, const char *name, const char *descriptiveType):
			Driver(address, name, "serial", descriptiveType)
		{}

		virtual void set_baud(U32 set) = 0;

		virtual auto get_active_baud() -> U32 = 0;

		virtual void putc(unsigned char c) = 0;
		virtual void puts(const char *str) { while(*str) putc(*str); }
		virtual auto getc() -> unsigned char = 0;
		virtual void gets(char *buffer, U32 length) {
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

		virtual void bind_stdio() = 0;
	};
}
