#include "memory.hpp"

#include <common/format.hpp>
#include <common/MemoryPool.hpp>
#include <common/stdlib.hpp>

#include <common/debugUtils.hpp>
#include <kernel/memory/Page.hpp>
#include <kernel/memory/PagedPool.hpp>
#include <kernel/Spinlock.hpp>
#include <kernel/log.hpp>

#ifndef HAS_UNALIGNED_ACCESS
	extern "C" auto memset(void *dest, int value, size_t size) -> void* {
		#ifdef MEMORY_CHECKS
			log::print_debug("memset ", format::Hex64{dest}, size, "\n");
		#endif
		auto d = (char*)dest;
		while(--size) *d++ = value;
		return dest;
	}
#endif

namespace memory {
	U64 totalMemory = 0;
	Page *pageData = nullptr;
	size_t pageDataSize = 0;

	Spinlock lock("memory");
	
	LList<::memory::Page> freePages;

	memory::PagedPool<sizeof(size_t)> kernelHeap;
	// MemoryPool<32> *heap;

	void Page::clear() {
		bzero(physicalAddress, pageSize);
	}

	Page* _allocate_page() {
		if(freePages.size<1){
			return nullptr;
		}

		auto page = freePages.pop_front();
		if(!page) return nullptr;

		page->isAllocated = true;
		page->clear();

		asm volatile("" ::: "memory");

		return page;
	}

	Page* _allocate_pages(U32 count){
		if(count<1) return nullptr;
		if(count==1) return _allocate_page();

		for(auto page=freePages.head; page; page=page->next){
			U32 needed = count-1;
			for(auto checkPage=page; needed&&checkPage->hasNextPage&&!(checkPage+1)->isAllocated; checkPage++,needed--);
			// log::print("searched\n");

			if(needed==0){
				U32 needed = count;
				U32 popped = 0;

				for(auto reservePage=page; needed; reservePage++, needed--){
					popped++;
					freePages.pop(*reservePage);
					reservePage->isAllocated = true;
					reservePage->clear();
				}

				return page;
			}
		}

		// log::print("didn't get pages\n");
		return nullptr;
	}

	void _free_page(Page &page) {
		page.isAllocated = false;

		asm volatile("" : "=m" (page)); //ensure any writes to page are definitely finished before we finally let it go ¯\_(ツ)_/¯
		
		freePages.push_back(page);
	}

	void* _kmalloc(size_t size) {
		// log::Section section("kmalloc");

		#ifdef MEMORY_CHECKS
			log::Section section("kmalloc ", size);
			debug_llist(kernelHeap.availableBlocks, "availableBlocks in kmalloc 0");
		#endif

		void *address;

		address = kernelHeap.malloc(size);
		// void *address = heap->malloc(size);

		if(!address){
			log::print_warning("Warning: Out of memory allocating ", size, "B for kernel");
		}

		#ifdef MEMORY_CHECKS
			log::print_debug("kmalloc ", size, " @ ", address);
			debug_llist(kernelHeap.availableBlocks, "availableBlocks after kmalloc");
		#endif

		_check_dangerous_address(address, (U8*)address+size-1);

		return address;
	}

	void _kfree(void *address) {
		// log::Section section("kfree");

		#ifdef MEMORY_CHECKS
			log::print_debug("kfree ", address);
		#endif

		if(!address) return;
		
		kernelHeap.free(address);
		// heap->free(address);
	}

	void _check_dangerous_address(void *from, void *to) {
		for(auto block=kernelHeap.availableBlocks.head; block; block=block->next) {
			if(to>block&&from<&block->_data+block->size){
				log::print_error("Error: DANGEROUS ADDRESS ", from, " -> ", (U8*)to-1, " overlaps block ", block, " -> ", &block->_data+block->size-1);
			}
		}
	}

	void Transaction::lock() {
		memory::lock.lock();
	}

	void Transaction::unlock() {
		memory::lock.unlock();
	}
}
