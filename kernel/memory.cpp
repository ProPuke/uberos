#include "memory.hpp"

#include <common/debugUtils.hpp>
#include <common/format.hpp>
#include <common/MemoryPool.hpp>
#include <common/stdlib.hpp>

#include <kernel/kernel.h>
#include <kernel/Lock.hpp>
#include <kernel/Log.hpp>
#include <kernel/memory/Page.hpp>
#include <kernel/memory/PagedPool.hpp>

static Log log("mem");

#ifndef HAS_UNALIGNED_ACCESS
	extern "C" auto memset(void *dest, int value, size_t size) -> void* {
		#ifdef MEMORY_CHECKS
			debug::trace("memset ", to_string(dest), size, "\n");
		#endif
		auto d = (char*)dest;
		while(--size) *d++ = value;
		return dest;
	}
#endif

extern U8 __start, __end;

namespace memory {
	constinit U64 totalMemory = 0;

	constinit size_t lowMemorySize = 1024*4;
	constinit void *lowMemory = &__start-1024*4;

	constinit size_t stackSize = KERNEL_STACK_SIZE;
	constinit void *stack = &__end;

	constinit void *heap = &__end+KERNEL_STACK_SIZE; // start the heap after the stack
	constinit size_t heapSize = 1*1024*1024; // default to 1MB (hopefully overridden at boot)

	Lock<LockType::flat> lock("memory");
	
	LList<::memory::Page> freePages;
	auto needsCompacting = false;

	memory::PagedPool<sizeof(size_t)> kernelHeap;
	// MemoryPool<32> *heap;

	auto get_used_heap() -> size_t {
		return kernelHeap.used;
	}

	auto get_available_heap() -> size_t {
		return kernelHeap.available;
	}

	// for allocating and freeing pages we do the dumb/fast thing by default - allocate from the front, slap frees on the end
	// periodically we may want to compact() to clean that up, and sort and consolidate fragmented free page entries

	Page* _allocate_pages(U32 count){
		if(count<1) return nullptr;

		for(auto page=freePages.head; page; page=page->next){
			if(page->count>=count){
				const auto remainingPages = count-page->count;
				if(remainingPages>0){
					auto &nextPages = page->get_offset_page(count);
					nextPages.count = remainingPages;
					freePages.insert_after(*page, nextPages);
				}
				freePages.pop(*page);

				page->clear();
				return page;
			}
		}

		if(needsCompacting){
			// if we failed, but we're compacted, clean it all up and try once again
			_compact();
			return _allocate_pages(count);
		}

		// debug::trace("didn't get pages\n");
		return nullptr;
	}

	void _free_pages(Page &page, U32 count) {
		asm volatile("" : "=m" (page)); //ensure any writes to page are definitely finished before we finally let it go ¯\_(ツ)_/¯

		page.count = count;

		freePages.push_back(page);
		needsCompacting = true;
	}

	void* _kmalloc(size_t size) {
		// auto section = log.section("kmalloc");

		#ifdef MEMORY_CHECKS
			auto section = log.section("kmalloc ", size);
			debug_llist(kernelHeap.availableBlocks, "availableBlocks in kmalloc 0");
		#endif

		void *address;

		address = kernelHeap.malloc(size);
		// void *address = heap->malloc(size);

		if(!address){
			log.print_warning("Warning: Out of memory allocating ", size, "B for kernel");
		}

		#ifdef MEMORY_CHECKS
			debug::trace("kmalloc ", size, " @ ", address);
			debug_llist(kernelHeap.availableBlocks, "availableBlocks after kmalloc");
		#endif

		_check_dangerous_address(address, (U8*)address+size-1);

		return address;
	}

	void _kfree(void *address) {
		// auto section = log.section("kfree");

		#ifdef MEMORY_CHECKS
			debug::trace("kfree ", address);
		#endif

		if(!address) return;

		kernelHeap.free(address);
		// heap->free(address);
	}

	void _check_dangerous_address(void *from, void *to) {
		for(auto block=kernelHeap.availableBlocks.head; block; block=block->next) {
			if(to>block&&from<&block->_data+block->size){
				log.print_error("Error: DANGEROUS ADDRESS ", from, " -> ", (U8*)to-1, " overlaps block ", block, " -> ", &block->_data+block->size-1);
			}
		}
	}

	void _compact() {
		if(!needsCompacting) return;

		{ // sort pages by address (by building a new sorted list, and then replacing with it)
			LList<::memory::Page> sortedPages;

			while(auto page = freePages.pop_front()){
				for(auto other=sortedPages.head; page; page=page->next){
					if(other>page){
						sortedPages.insert_before(*other, *page);
						goto sorted;
					}
				}

				sortedPages.push_back(*page);
				sorted:
				;
			}

			freePages = sortedPages;
		}

		// consolidate consecutive pages
		for(auto page=freePages.head; page&&page->next; page=page->next){
			if(page->next==&page->get_offset_page(page->count)){
				page->count += page->next->count;
				freePages.pop(*page->next);
			}
		}

		needsCompacting = false;
	}

	void Transaction::lock() {
		memory::lock.lock();
	}

	void Transaction::unlock() {
		memory::lock.unlock();
	}

	void init() {
		{ // all heap as a single page entry at the start
			auto &page = *(Page*)heap;
			page.count = ((UPtr)heap+heapSize)/pageSize;
			if(page.count>0){
				freePages.push_back(page);
			}
		}
	}
}
