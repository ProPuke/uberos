#pragma once

#include <kernel/logging.hpp>
#include <kernel/memory.hpp>

#include <common/LList.hpp>
#include <common/MemoryPool.hpp>

//TODO:overload compact() and free whole pages when they're no longer in use

namespace memory {
	template <unsigned alignment>
	struct PagedPool: MemoryPool<alignment> {
		typedef MemoryPool<alignment> Super;

		/**/ PagedPool(void *heap, size_t heapSize):
			Super(heap, heapSize)
		{}

		/**/ PagedPool():
			Super(nullptr, 0)
		{}

		void* malloc(size_t size) {
			#ifdef MEMORY_CHECKS
				logging::Section section("PagedPool::malloc ", size);
			#endif

			const auto blockHeaderSize = offsetof(MemoryPoolBlock, MemoryPoolBlock::_data);

			unsigned requiredSize = align(size, alignment);

			if(requiredSize>this->available){
				if(blockHeaderSize+requiredSize>=memory::pageSize){
					const auto pageCount = (blockHeaderSize+requiredSize+memory::pageSize-1)/memory::pageSize;
					#ifdef MEMORY_CHECKS
						debug::trace("try to add ", pageCount, " pages\n");
					#endif
					if(!add_pages(pageCount)) {
						#ifdef MEMORY_CHECKS
							debug::trace("could not allocate ", pageCount, " pages \n");
						#endif
						return nullptr;
					}

				}else{
					#ifdef MEMORY_CHECKS
						debug::trace("try to add page\n");
					#endif
					if(!add_page()) {
						#ifdef MEMORY_CHECKS
							debug::trace("could not allocate page \n");
						#endif
						return nullptr;
					}
				}
			}

			#ifdef MEMORY_CHECKS
				debug::trace("try to malloc 2 ", requiredSize, "\n");
			#endif

			void *result;

			do{
				result = Super::malloc(requiredSize);

				if(!result){
					#ifdef MEMORY_CHECKS
						debug::trace("could not allocate ", requiredSize, "\n");
					#endif
					if(blockHeaderSize+requiredSize>=memory::pageSize){
						#ifdef MEMORY_CHECKS
							const auto sizeBefore = this->available;
						#endif
						const auto pageCount = (blockHeaderSize+requiredSize+memory::pageSize-1)/memory::pageSize;
						#ifdef MEMORY_CHECKS
							debug::trace("trying to allocate ", pageCount, " pages\n");
						#endif
						if(!add_pages(pageCount)) {
							#ifdef MEMORY_CHECKS
								debug::trace("could not allocate ", pageCount, " pages \n");
							#endif
							return nullptr;
						}
						#ifdef MEMORY_CHECKS
							debug::trace("managed to allocate ", this->available-sizeBefore, "\n");
						#endif

					}else{
						if(!add_page()) {
							#ifdef MEMORY_CHECKS
								debug::trace("could not allocate page \n");
							#endif
							return nullptr;
						}
					}
				}
			}while(!result);

			#ifdef MEMORY_CHECKS
				debug::trace("returned memory ", result, "\n");
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
