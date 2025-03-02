#pragma once

#include "memory.hpp"

#include <kernel/memory/Page.hpp>

#include <common/types.hpp>

namespace memory {
	inline auto _get_memory_page(void *address) -> Page& {
		return *(Page*)((UPtr)address/memory::pageSize*memory::pageSize);
	}
}
