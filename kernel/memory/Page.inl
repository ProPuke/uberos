#pragma once

#include "Page.hpp"

#include <kernel/memory.hpp>

namespace memory {
	inline void Page::clear() {
		bzero(this, count*pageSize);
	}

	inline auto Page::get_offset_page_physical(U32 ahead) -> Physical<Page> {
		return Physical<Page>{physical.address+memory::pageSize*ahead};
	}
}
