#include "timer.hpp"

#include <kernel/Log.hpp>

static Log log("arch::x86::timer");

namespace arch {
	namespace x86 {
		namespace timer {
			void init() {
				auto section = log.section("init...");
			}
		}
	}
}

namespace timer {
	void _schedule_update(U32 usecs) {
		; //TODO
	}

	U32 now() {
		return 0; // TODO
	}

	U64 now64() {
		return 0; // TODO
	}

	void wait(U32 usecs) {
		// TODO
	}
}
