#pragma once

#include <common/types.hpp>
#include <common/LList.hpp>

// template <unsigned alignment>
// struct MemoryPool;

namespace memory {
	struct Page: LListItem<Page> {
		bool hasNextPage:1 = false;

		void clear();
		auto next_page() -> Page&;
	};
}

#include "Page.inl"
