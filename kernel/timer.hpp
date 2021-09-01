#pragma once

#include <common/types.hpp>

namespace timer {
	U32 now();
	U64 now64();
	
	void udelay(U32 usecs);
}
