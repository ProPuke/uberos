#pragma once

#include <kernel/memory.hpp>

namespace arch {
	namespace raspi {
		namespace memory {
			void init();
		}
	}

	static inline const U32 stackSize = ::memory::pageSize;
	// static inline const U32 heapSize = 1024*1024;
}
