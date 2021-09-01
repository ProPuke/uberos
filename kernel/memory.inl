#pragma once

#include "memory.hpp"

#include "memory/Page.hpp"
#include <common/types.hpp>

namespace memory {
	inline Page* _get_memory_page(void *address){
		return &pageData[(size_t)address/sizeof(Page)];
	}
}
