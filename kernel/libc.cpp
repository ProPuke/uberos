#include "libc.hpp"

#include "stdio.hpp"

extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

extern "C" int raise(int signal) {
	stdio::print_info("RAISE SIGNAL ", signal);
	while(true);
}

namespace libc {
	void init() {
		stdio::Section section("libc::init");

		stdio::print_info_start();
			for(auto func=__init_array_start; func!=__init_array_end; func++) {
				stdio::print_inline('.');
				(*func)();
			}
		stdio::print_end();
	}
}
