#include <kernel/Log.hpp>

static Log log("libc");

extern "C" void _init();

extern "C" int raise(int signal) {
	log.print_info("RAISE SIGNAL ", signal);

	halt();
}

namespace libc {
	void init() {
		_init();
	}
}
