#include "memory.hpp"

#include <common/AllocationMask.hpp>
#include <common/debugUtils.hpp>
#include <common/format.hpp>
#include <common/MemoryPool.hpp>
#include <common/stdlib.hpp>

#include <kernel/kernel.h>
#include <kernel/Lock.hpp>
#include <kernel/Log.hpp>
#ifdef KERNEL_MMU
	#include <kernel/mmu.hpp>
#endif
#include <kernel/memory/Page.hpp>
#include <kernel/memory/PagedPool.hpp>
#include <kernel/panic.hpp>
#include <kernel/PhysicalPointer.hpp>

static Log log("mem");

#ifndef HAS_UNALIGNED_ACCESS
	extern "C" auto memset(void *dest, int value, size_t size) -> void* {
		#ifdef MEMORY_CHECKS
			debug::trace("memset ", to_string(dest), size, "\n");
		#endif
		auto d = (char*)dest;
		while(size--) *d++ = value;
		return dest;
	}
#endif

extern U8 __start, __end;
extern U8 __text_start, __text_end;
extern U8 __rodata_start, __rodata_end;
extern U8 __data_start, __data_end;
extern U8 __bss_start, __bss_end;

namespace memory {
	constinit U64 totalMemory = 0;

	constinit IdentityMapped<void> lowMemory{(UPtr)&__start - LOW_MEMORY_SIZE};
	constinit size_t lowMemorySize = LOW_MEMORY_SIZE;

	constinit IdentityMapped<void> code{(UPtr)&__text_start};
	/*constinit*/ size_t codeSize = (UPtr)&__text_end-(UPtr)&__text_start;

	constinit IdentityMapped<void> constants{(UPtr)&__rodata_start};
	/*constinit*/ size_t constantsSize = (UPtr)&__rodata_end-(UPtr)&__rodata_start;

	constinit IdentityMapped<void> initialisedData{(UPtr)&__data_start};
	/*constinit*/ size_t initialisedDataSize = (UPtr)&__data_end-(UPtr)&__data_start;

	constinit IdentityMapped<void> uninitialisedData{(UPtr)&__bss_start};
	/*constinit*/ size_t uninitialisedDataSize = (UPtr)&__bss_end-(UPtr)&__bss_start;

	constinit IdentityMapped<void> stack{(UPtr)&__end};
	constinit size_t stackSize = KERNEL_STACK_SIZE;

	constinit Physical<void> heap{(UPtr)&__end + KERNEL_STACK_SIZE};
	constinit size_t heapSize = 1*1024*1024; // default to 1MB (hopefully overridden at boot)

	#ifdef KERNEL_MMU
		void *virtualScratchRecord = nullptr; // a reusable virtual page, used for reading/writing physical regions
	#endif

	Lock<LockType::recursive> lock("memory");
	
	#ifdef KERNEL_MMU
		// a plain bitmask of used pages (as with an mmu we don't need physical addresses to be sequential, only the virtual we map)
		AllocationMask pageAllocations{nullptr, 0};
	#else
		// list of sequential page chunks, so that we can request large sequential ranges without an mmu
		LList<::memory::Page> freePages;
		auto needsCompacting = false;
	#endif

	memory::PagedPool<sizeof(size_t)> kernelHeap;
	// MemoryPool<32> *heap;

	auto get_used_heap() -> size_t {
		return kernelHeap.used;
	}

	auto get_available_heap() -> size_t {
		return kernelHeap.available;
	}

	auto get_heap_block_count() -> U32 {
		return kernelHeap.availableBlocks.length();
	}

	namespace {
		auto physical_to_index(Physical<void> physical) -> UPtr {
			return (physical.address - heap.address) / pageSize;
		}
		auto index_to_physical(UPtr index) -> Physical<void> {
			return heap + index * pageSize;
		}
	}

	// for allocating and freeing pages we do the dumb/fast thing by default - allocate from the front, slap frees on the end
	// periodically we may want to compact() to clean that up, and sort and consolidate fragmented free page entries

	Page* _allocate_pages(U32 count){
		if(count<1) return nullptr;

		#ifdef KERNEL_MMU
			auto kernelTransaction = mmu::kernel::transaction();

			for(auto remainingCount = count; remainingCount>1; remainingCount--){
				auto physical = index_to_physical(TRY_RESULT_OR_RETURN(pageAllocations.claim(), nullptr)); // TODO: unmap previous pages and addresses on fail/nullptr return
				(Page*)kernelTransaction.map_physical_high(physical, {});
			}

			auto physical = index_to_physical(TRY_RESULT_OR_RETURN(pageAllocations.claim(), nullptr)); // TODO: unmap previous pages and addresses on fail/nullptr return
			auto page = (Page*)kernelTransaction.map_physical_high(physical, {});

			page->physical = physical.as_type<Page>();
			page->count = count;

			return page;
		#else
			{
				#ifdef KERNEL_MMU
					auto kernelTransaction = mmu::kernel::transaction();
				#endif

				for(auto page=freePages.head; page; page=page->next){
					if(page->count>=count){
						const auto remainingPages = page->count-count;
						if(remainingPages>0){
							auto nextPagesPhysical = page->get_offset_page_physical(count);
							#ifdef KERNEL_MMU
								auto &nextPages = *(Page*)kernelTransaction.map_physical_high(nextPagesPhysical, {});
							#else
								auto &nextPages = *(Page*)nextPagesPhysical.address;
							#endif
							nextPages.physical = nextPagesPhysical;
							nextPages.count = remainingPages;
							freePages.insert_after(*page, nextPages);
						}
						page->count = count;
						freePages.pop(*page);

						return page;
					}
				}
			}

			if(needsCompacting){
				// if we failed, but we're compacted, clean it all up and try once again
				_compact();
				return _allocate_pages(count);
			}
		#endif

		// debug::trace("didn't get pages\n");
		return nullptr;
	}

	void _free_pages(Page &page, U32 count) {
		asm volatile("" : "=m" (page)); //ensure any writes to page are definitely finished before we finally let it go ¯\_(ツ)_/¯

		#ifdef KERNEL_MMU
			page.physical = mmu::kernel::transaction().get_physical(&page).as_type<memory::Page>();
			pageAllocations.release(physical_to_index(page.physical));
		#else
			#ifdef KERNEL_MMU
				page.physical = mmu::kernel::transaction().get_physical(&page).as_type<memory::Page>();
			#else
				page.physical = Physical<Page>{(UPtr)&page};
			#endif
			page.count = count;

			freePages.push_back(page);
			needsCompacting = true;
		#endif
	}

	void* _allocate(size_t size) {
		if(size<0) return nullptr;

		// auto section = log.section("allocate");

		#ifdef MEMORY_CHECKS
			auto section = log.section("allocate ", size);
			debug();
		#endif

		void *address;

		address = kernelHeap.malloc(size);
		// void *address = heap->malloc(size);

		if(!address){
			log.print_warning("Warning: Out of memory allocating ", size, "B for kernel");
		}

		#ifdef MEMORY_CHECKS
			debug::trace("allocate ", size, " @ ", address);
			debug();
		#endif

		_check_dangerous_address(address, (U8*)address+size-1);

		return address;
	}

	void _free(void *address) {
		// auto section = log.section("free");

		#ifdef MEMORY_CHECKS
			debug::trace("free ", address);
		#endif

		if(!address) return;

		kernelHeap.free(address);
		// heap->free(address);
	}

	void _check_dangerous_address(void *from, void *to) {
		for(auto block=kernelHeap.availableBlocks.head; block; block=block->next) {
			if(to>block&&from<&block->_data+block->size){
				// panic::panic();
				log.print_error("Error: DANGEROUS ADDRESS ", from, " -> ", (U8*)to-1, " overlaps block ", block, " -> ", &block->_data+block->size-1);
			}
		}
	}

	void _compact() {
		#ifndef KERNEL_MAPPING
			// while(needsCompacting){
			// 	needsCompacting = false;

			// 	{ // sort pages by address (by building a new sorted list, and then replacing with it)
			// 		LList<::memory::Page> sortedPages;

			// 		while(auto page = freePages.pop_front()){
			// 			for(auto other=sortedPages.head; page; page=page->next){
			// 				if(other->physical>page->physical){
			// 					sortedPages.insert_before(*other, *page);
			// 					goto sorted;
			// 				}
			// 			}

			// 			sortedPages.push_back(*page);
			// 			sorted:
			// 			;
			// 		}

			// 		freePages = sortedPages;
			// 	}

			// 	// consolidate consecutive pages
			// 	for(auto page=freePages.head; page&&page->next; page=page->next){
			// 		if(page->next->physical.address==page->get_offset_page_physical(page->count).address){
			// 			page->count += page->next->count;
			// 			freePages.pop(*page->next);
			// 			#ifdef KERNEL_MMU
			// 				mmu::kernel::unmap_virtual_high(&page->next, pageSize * page->count); // NOTE: This should ONLY free memory, NEVER allocate it (we don't want an infinite compact loop)
			// 			#endif
			// 		}
			// 	}
			// }
		#endif
	}

	void Transaction::lock() {
		memory::lock.lock();
	}

	void Transaction::unlock() {
		memory::lock.unlock();
	}

	void init() {
		{ // all heap as a single page entry at the start
			#ifdef KERNEL_MMU
				auto totalPageCount = heapSize/pageSize;
				assert(totalPageCount>1);
				auto maskSizeNeeded = AllocationMask::size_required(totalPageCount);
				const auto allocationPagesNeeded = (maskSizeNeeded+pageSize-1)/pageSize;

				assert(allocationPagesNeeded<totalPageCount);
				totalPageCount -= allocationPagesNeeded; //subtract space for the allocation mask

				auto allocationData = mmu::kernel::transaction().map_physical_high(heap, allocationPagesNeeded, {});
				heap += allocationPagesNeeded*pageSize;
				heapSize -= allocationPagesNeeded*pageSize;

				pageAllocations = AllocationMask{(Reg*)allocationData, totalPageCount};

			#else
				auto &page = *(Page*)heap.address;
				page.physical = heap.as_type<memory::Page>();
				page.count = heapSize/pageSize;
				if(page.count>0){
					freePages.push_back(page);
				}
			#endif
		}

		#ifdef KERNEL_MMU
			virtualScratchRecord = mmu::kernel::transaction().map_physical_high({0x00}, {.caching=mmu::Caching::uncached});
		#endif
	}

	void debug() {
		debug_llist(kernelHeap.availableBlocks);
		#ifndef KERNEL_MMU
			debug_llist(freePages);
		#endif
	}

	auto read_physical(Physical<void> physicalAddress, UPtr size) -> Box<U8> {
		Box<U8> buffer = new U8[size];

		#ifdef KERNEL_MMU
			auto bufferOffset = 0u;
			auto pageAddress = physicalAddress.address/pageSize*pageSize;
			auto transaction = mmu::kernel::transaction();

			{
				const auto offset = physicalAddress.address-pageAddress;
				const auto length = min(size, pageSize-offset); // how much of the first page overlaps the read range
				transaction.set_virtual_target(virtualScratchRecord, {pageAddress}, pageSize);
				memcpy(&buffer[bufferOffset], (U8*)virtualScratchRecord+offset, length);
				bufferOffset += length;
			}

			for(pageAddress+=pageSize; pageAddress<physicalAddress.address+size; pageAddress+=pageSize){
				transaction.set_virtual_target(virtualScratchRecord, {pageAddress}, pageSize);
				memcpy(&buffer[bufferOffset], virtualScratchRecord, min(pageSize, size-bufferOffset));
				bufferOffset += pageSize;
			}
		#else
			memcpy(&buffer[0], (void*)physicalAddress.address, size);
		#endif

		return buffer;
	}

	void write_physical(Physical<void> physicalAddress, U8 *data, UPtr size) {
		#ifdef KERNEL_MMU
			auto dataOffset = 0u;
			auto pageAddress = physicalAddress.address/pageSize*pageSize;
			auto transaction = mmu::kernel::transaction();

			{
				const auto offset = physicalAddress.address-pageAddress;
				const auto length = pageSize-offset; // how much of the first page overlaps the read range
				transaction.set_virtual_target(virtualScratchRecord, {pageAddress}, pageSize);
				memcpy((U8*)virtualScratchRecord+offset, &data[dataOffset], length);
				dataOffset += length;
			}

			for(pageAddress+=pageSize; pageAddress<physicalAddress.address+size; pageAddress+=pageSize){
				transaction.set_virtual_target(virtualScratchRecord, {pageAddress}, pageSize);
				memcpy(virtualScratchRecord, &data[dataOffset], min(pageSize, size-dataOffset));
				dataOffset += pageSize;
			}
		#else
			memcpy((void*)physicalAddress.address, data, size);
		#endif
	}
}
