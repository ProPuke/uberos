#pragma once

#include <kernel/memory.hpp>
#include <kernel/mmu.hpp>

#include <common/types.hpp>

namespace mmu {
	struct TableDescriptor;
	
	struct MemoryMapping {
		TableDescriptor *initialTable = nullptr;
		U32 pageCount = 0;

		/**/ MemoryMapping(bool allocate = true);
		/**/~MemoryMapping();

		public:

		void allocate();

		void clear();
		void* add_pages(U32 count, RegionType regionType);
		void* add_mapping(void *address, U32 pageCount, RegionType regionType);
		void* add_mapping(void *addressStart, void *addressEnd, RegionType regionType);
		U32 get_page_count() { return pageCount; }
		U64 get_memory_size() { return get_page_count() * memory::pageSize; }
	};
}
