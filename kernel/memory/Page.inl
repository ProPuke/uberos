#pragma once

#include "Page.hpp"

#include <kernel/memory.hpp>

namespace memory {
	inline void Page::clear() {
		bzero(this, pageSize);
	}

	inline auto Page::get_offset_page(U32 ahead) -> Page& {
		return *(Page*)((UPtr)this+memory::pageSize*ahead);
	}
}
