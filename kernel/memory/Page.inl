#pragma once

#include "Page.hpp"

#include <kernel/memory.hpp>

namespace memory {
	inline void Page::clear() {
		bzero(this, pageSize);
	}

	inline auto Page::next_page() -> Page& {
		return *(Page*)((UPtr)this+memory::pageSize);
	}
}
