#pragma once

#include <kernel/memory.hpp>

namespace memory {
	namespace arch {
		namespace raspi {
			void init();
		}
	}

	static inline const U32 stackSize = ::memory::pageSize;
	// static inline const U32 heapSize = 1024*1024;
}
