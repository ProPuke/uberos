#pragma once

#include <common/types.hpp>

#include <kernel/memory.hpp>

namespace mmu {
	namespace arch {
		namespace arm64 {
			void init();

			void enable();
			void disable();

			struct Block {
				void *physical_address;
				U64 size;
			};

			struct MemoryMapping {
				void *initialTable;
				U32 pageCount;

				/**/ MemoryMapping();
				/**/~MemoryMapping();

				public:

				void clear();
				bool add_pages(U32 count);
				U32 get_page_count() { return pageCount; }
				U64 get_memory_size() { return get_page_count() * memory::pageSize; }
			};

			void set_userspace_mapping(MemoryMapping &memoryMapping);
		}
	}
}
