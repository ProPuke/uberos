#pragma once

#include <common/types.hpp>
#include <common/LList.hpp>

// template <unsigned alignment>
// struct MemoryPool;

namespace memory {
	struct Page: LListItem<Page> {
		U32 count = 0; // how many consecutive free pages are there starting from here

		void clear();
		auto get_offset_page(U32 ahead=1) -> Page&;
	};
}

#include "Page.inl"
