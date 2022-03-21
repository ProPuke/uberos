#pragma once

#include <common/types.hpp>

namespace scheduler {
	void yield();
	
	void lock();
	void unlock();

	U32 get_total_thread_count();
	U32 get_active_thread_count();

	struct Guard {
		/**/ Guard() { lock(); }
		/**/~Guard() { unlock(); }

		/**/ Guard(const Guard&) = delete;
		Guard& operator=(const Guard&) = delete;
	};
}
