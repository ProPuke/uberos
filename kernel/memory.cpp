#include "memory.hpp"

#include "memory/Page.hpp"
#include "memory/PagedPool.hpp"
#include "Spinlock.hpp"
#include "stdio.hpp"
#include <common/debugUtils.hpp>
#include <common/MemoryPool.hpp>
#include <common/stdlib.hpp>

extern "C" void memset(U8 *address, U8 value, unsigned int size) {
	#ifdef MEMORY_CHECKS
		stdio::print_debug("memset ", (void*)address, size, "\n");
	#endif
	while(--size) *address++ = value;
}

namespace memory {
	U64 totalMemory = 0;
	Page *pageData = nullptr;
	size_t pageDataSize = 0;

	Spinlock lock("memory");
	
	LList<::memory::Page> freePages;

	memory::PagedPool<4> kernelHeap;
	// MemoryPool<32> *heap;

	void Page::clear() {
		bzero(physicalAddress, pageSize);
	}

	Page* allocate_page() {
		Spinlock_Guard _lock(lock, __FUNCTION__);
		return _allocate_page();
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

	Page* allocate_pages(U32 count){
		Spinlock_Guard _lock(lock, __FUNCTION__);

		return _allocate_pages(count);
	}

	Page* _allocate_pages(U32 count){
		if(count<1) return nullptr;
		if(count==1) return _allocate_page();

		for(auto page=freePages.head; page; page=page->next){
			U32 needed = count-1;
			for(auto checkPage=page; needed&&checkPage->hasNextPage&&!(checkPage+1)->isAllocated; checkPage++,needed--);
			// stdio::print("searched\n");

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

		// stdio::print("didn't get pages\n");
		return nullptr;
	}

	void free_page(Page &page) {
		Spinlock_Guard _lock(lock, __FUNCTION__);

		return _free_page(page);
	}

	void _free_page(Page &page) {
		page.isAllocated = false;

		asm volatile("" : "=m" (page)); //ensure any writes to page are definitely finished before we finally let it go ¯\_(ツ)_/¯
		
		freePages.push_back(page);
	}

	//TODO
	// Page* get_memory_page(void *address) {
	// }

	void* kmalloc(size_t size) {
		// stdio::Section section("kmalloc");
		Spinlock_Guard _lock(lock, __FUNCTION__);

		#ifdef MEMORY_CHECKS
			stdio::Section section("kmalloc ", size);
			debug_llist(kernelHeap.availableBlocks, "availableBlocks in kmalloc 0");
		#endif

		void *address;

		address = kernelHeap.malloc(size);
		// void *address = heap->malloc(size);

		if(!address){
			stdio::print_warning("Warning: Out of memory allocating ", size, "B for kernel");
		}

		#ifdef MEMORY_CHECKS
			stdio::print_debug("kmalloc ", size, " @ ", address, "\n");
			debug_llist(kernelHeap.availableBlocks, "availableBlocks after kmalloc");
		#endif

		check_dangerous_address(address, (U8*)address+size-1);

		return address;
	}

	void kfree(void *address) {
		// stdio::Section section("kfree");
		Spinlock_Guard _lock(lock, __FUNCTION__);

		#ifdef MEMORY_CHECKS
			stdio::print_debug("kfree ", address, "\n");
		#endif

		if(!address) return;
		
		kernelHeap.free(address);
		// heap->free(address);
	}

	void check_dangerous_address(void *from, void *to) {
		for(auto block=kernelHeap.availableBlocks.head; block; block=block->next) {
			if(to>block&&from<&block->_data+block->size){
				stdio::print_error("Error: DANGEROUS ADDRESS ", from, " -> ", (U8*)to-1, " overlaps block ", block, " -> ", &block->_data+block->size-1);
			}
		}
	}
}

void* operator new(size_t size) noexcept { return memory::kmalloc(size); }
void* operator new[](size_t size) noexcept { return memory::kmalloc(size); }

void operator delete(void* p) noexcept { memory::kfree(p); }
void operator delete(void* p, size_t) noexcept { memory::kfree(p); }
