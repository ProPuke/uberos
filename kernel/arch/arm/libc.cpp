#include <kernel/Log.hpp>

static Log log("libc");

extern void (*__preinit_array_start [])() __attribute__((weak));
extern void (*__preinit_array_end [])() __attribute__((weak));
extern void (*__init_array_start [])() __attribute__((weak));
extern void (*__init_array_end [])() __attribute__((weak));

extern "C" int raise(int signal) {
	log.print_info("RAISE SIGNAL ", signal);
	while(true);
}

namespace libc {
	void init() {
		auto section = log.section("libc::init");

		log.print_info_start();
			for(auto func=__preinit_array_start; func!=__preinit_array_end; func++) {
				log.print_inline('.');
				(*func)();
			}

			for(auto func=__init_array_start; func!=__init_array_end; func++) {
				log.print_inline('.');
				(*func)();
			}
		log.print_end();
	}
}
