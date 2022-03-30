#pragma once

#include <kernel/Driver.hpp>
#include <kernel/stdio.hpp>

#include <functional>

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

				if(c=='\r'||c=='\n') break;
				
				putc(c);
				buffer[i] = c;
			}

			putc('\n');
			buffer[i] = '\0';
		}

		void bind_stdio() {
			if(state!=State::enabled) return;

			stdio::bind(this,
				[](void *self, unsigned char c) { return ((Serial*)self)->putc(c); },
				[](void *self) { return ((Serial*)self)->getc(); },
				[](void *self, const char *str) { return ((Serial*)self)->puts(str); },
				[](void *self, char *buffer, U32 length) { return ((Serial*)self)->gets(buffer, length); }
			);
		}
	};
}
