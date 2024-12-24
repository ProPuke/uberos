#pragma once

#include <common/types.hpp>

namespace scheduler {
	void push_resolution_requirement(U32);
	void pop_resolution_requirement();

	void init();

	void yield();
	
	void lock();
	void unlock();

	struct Guard {
		/**/ Guard() { lock(); }
		/**/~Guard() { unlock(); }

		/**/ Guard(const Guard&) = delete;
		Guard& operator=(const Guard&) = delete;
	};
}
