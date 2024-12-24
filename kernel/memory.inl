#pragma once

#include "memory.hpp"

#include <kernel/memory/Page.hpp>

#include <common/types.hpp>

namespace memory {
	inline auto _get_memory_page(void *address) -> Page& {
		return pageData[(size_t)address/sizeof(Page)];
	}
}
