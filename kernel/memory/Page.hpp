#pragma once

#include <common/types.hpp>
#include <common/LList.hpp>

#include <kernel/PhysicalPointer.hpp>

// template <unsigned alignment>
// struct MemoryPool;

namespace memory {
	struct Page: LListItem<Page> {
		Physical<Page> physical;
		U32 count; // how many consecutive free pages are there starting from here

		void clear();
		auto get_offset_page_physical(U32 ahead=1) -> Physical<Page>;
	};
}

#include "Page.inl"
