#include "exceptions.hpp"

#include <common/types.hpp>
#include <atomic>

namespace exceptions {
	// std::atomic<U32> _lock_depth = 0;
	volatile int _lock_depth = 0;
}
