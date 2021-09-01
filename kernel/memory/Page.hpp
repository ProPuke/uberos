#pragma once

#include <common/types.hpp>
#include <common/LList.hpp>

// template <unsigned alignment>
// struct MemoryPool;

namespace memory {
	struct Page: LListItem<Page> {
		/**/ Page(void *physicalAddress):
			physicalAddress(physicalAddress),
			isAllocated(false),
			isKernel(false),
			hasNextPage(false)
		{}

		U32 virtualAddress;
		void *physicalAddress;

		bool isAllocated:1; //NOTE: this isn't locked, so may be temporarily incorrect if interrupted just as inserting/removing pages
		bool isKernel:1;
		bool hasNextPage:1;

		void clear();
	};
}
