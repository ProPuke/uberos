#include "Stdout.hpp"

#include <cstdio>
#include <unistd.h>

namespace driver {
	namespace serial {
		/**/ Stdout:: Stdout(U64 address, const char *name):
			Serial(address, name, "serial port")
		{}

		auto Stdout::_on_start() -> bool {
			return true;
		}

		auto Stdout::_on_stop() -> bool {
			return true;
		}

		void Stdout::set_baud(U32 set) {
			return; // not supported
		}

		auto Stdout::get_active_baud() -> U32 {
			return 0; // not supported
		}

		void Stdout::putc(char c) {
			std::putchar(c);
		}
		void Stdout::puts(const char *str) {
			std::puts(str);
		}
		auto Stdout::peekc() -> char {
			timeval tv {0, 0};
			fd_set set;
			FD_ZERO(&set);
			FD_SET(STDIN_FILENO, &set);
			select(STDIN_FILENO+1, &set, nullptr, nullptr, &tv);

			if(!FD_ISSET(0, &set)) return 0;

			return getc();
		}
		auto Stdout::getc() -> char {
			std::getchar();
		}
	}
}
