#pragma once

#include "../memory.hpp"
#include "../stdio.hpp"
#include <common/LList.hpp>
#include <common/MemoryPool.hpp>

//TODO:overload compact() and free whole pages when they're no longer in use

namespace memory {
	template <unsigned alignment>
	struct PagedPool: MemoryPool<alignment> {
		typedef MemoryPool<alignment> Super;

		/**/ PagedPool():
			Super(nullptr, 0)
		{}

		void* malloc(size_t size) {
			#ifdef MEMORY_CHECKS
				stdio::Section section("PagedPool::malloc ", size);
			#endif

			const auto blockHeaderSize = offsetof(MemoryPoolBlock, MemoryPoolBlock::_data);

			if(size>this->available){
				if(blockHeaderSize+size>=memory::pageSize){
					const auto pageCount = (blockHeaderSize+size+memory::pageSize-1)/memory::pageSize;
					#ifdef MEMORY_CHECKS
						stdio::print_debug("try to add ", pageCount, " pages\n");
					#endif
					if(!add_pages(pageCount)) {
						#ifdef MEMORY_CHECKS
							stdio::print_debug("could not allocate ", pageCount, " pages \n");
						#endif
						return nullptr;
					}

				}else{
					#ifdef MEMORY_CHECKS
						stdio::print_debug("try to add page\n");
					#endif
					if(!add_page()) {
						#ifdef MEMORY_CHECKS
							stdio::print_debug("could not allocate page \n");
						#endif
						return nullptr;
					}
				}
			}

			#ifdef MEMORY_CHECKS
				stdio::print_debug("try to malloc 2 ", size, "\n");
			#endif

			void *result;

			do{
				result = Super::malloc(size);

				if(!result){
					#ifdef MEMORY_CHECKS
						stdio::print_debug("could not allocate ", size, "\n");
					#endif
					if(blockHeaderSize+size>=memory::pageSize){
						#ifdef MEMORY_CHECKS
							const auto sizeBefore = this->available;
						#endif
						const auto pageCount = (blockHeaderSize+size+memory::pageSize-1)/memory::pageSize;
						#ifdef MEMORY_CHECKS
							stdio::print_debug("trying to allocate ", pageCount, " pages\n");
						#endif
						if(!add_pages(pageCount)) {
							#ifdef MEMORY_CHECKS
								stdio::print_debug("could not allocate ", pageCount, " pages \n");
							#endif
							return nullptr;
						}
						#ifdef MEMORY_CHECKS
							stdio::print_debug("managed to allocate ", this->available-sizeBefore, "\n");
						#endif

					}else{
						if(!add_page()) {
							#ifdef MEMORY_CHECKS
								stdio::print_debug("could not allocate page \n");
							#endif
							return nullptr;
						}
					}
				}
			}while(!result);

			#ifdef MEMORY_CHECKS
				stdio::print_debug("returned memory ", result, "\n");
			#endif
			
			return result;
		}

		bool add_page() {
			auto page = memory::_allocate_page();
			if(!page) return false;

			auto &block = *new ((MemoryPoolBlock*)page->physicalAddress) MemoryPoolBlock(memory::pageSize);
			this->claim_block(block);

			return true;
		}

		bool add_pages(U32 count) {
			auto newPages = memory::_allocate_pages(count);
			if(!newPages) return false;

			auto &block = *new ((MemoryPoolBlock*)newPages->physicalAddress) MemoryPoolBlock(memory::pageSize*count);
			this->claim_block(block);

			return true;
		}
	};
}
