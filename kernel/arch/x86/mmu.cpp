#include "mmu.hpp"

#include <kernel/arch/x86/cpuInfo.hpp>
#include <kernel/assert.hpp>
#include <kernel/memory.hpp>

namespace mmu {
	void init() {
		assert(::arch::x86::cpuInfo::get_features().pat, "PAT not supported by cpu");

		// we'll identity map the stack as low. This is fine - we won't need the stack while running from other mappings
		kernel::map_physical_low((void*)0x00, (UPtr)memory::stack+memory::stackSize, {.caching = Caching::uncached});
		kernel::set_virtual_options(memory::stack, memory::stackSize, {.caching = Caching::writeBack});
	}

	void activate(Mapping &mapping) {
		asm volatile(R"(
			// set directory
			mov cr3, %0

			// activate
			mov eax, cr0
			or eax, 0x80000000
			mov cr0, eax
		)"
			:
			: "r"(&mapping.directory)
			: "memory"
		);
	}

	void deactivate() {
		asm volatile(R"(
			mov eax, cr0
			or eax, ~0x80000000
			mov cr0, eax
		)"
			:
			:
			: "memory"
		);
	}

	Mapping kernelMapping;

	namespace kernel {
		auto map_physical_low(void *address, MapOptions options) -> void* {
			return kernelMapping.map_physical_low(address, options);
		}
		auto map_physical_low(void *address, UPtr size, MapOptions options) -> void* {
			return kernelMapping.map_physical_low(address, size, options);
		}
		auto map_physical_high(void *address, MapOptions options) -> void* {
			return kernelMapping.map_physical_high(address, options);
		}
		auto map_physical_high(void *address, UPtr size, MapOptions options) -> void* {
			return kernelMapping.map_physical_high(address, size, options);
		}

		void set_virtual_options(void *address, MapOptions options) {
			kernelMapping.set_virtual_options(address, options);
		}
		void set_virtual_options(void *address, UPtr size, MapOptions options) {
			kernelMapping.set_virtual_options(address, size, options);
		}
	}

	/**/ Mapping:: Mapping(){}

	/**/ Mapping::~Mapping() {
		clear();
	}

	void Mapping::clear() {
		for(auto directoryIndex=0;directoryIndex<=nextLowDirectoryIndex;directoryIndex++){
			auto &directoryEntry = directory.entry[directoryIndex];
			if(directoryEntry.isPresent){
				memory::Transaction().free_page_with_address(directoryEntry.get_table());
				directoryEntry.isPresent = false;
			}
		}

		nextLowDirectoryIndex = 0;
	}

	auto Mapping::map_physical_low(void *physicalAddress, MapOptions options) -> void* {
		// align physicalAddress and store remainder in offset
		auto physicalOffset = (UPtr)physicalAddress&0b111111111111;
		physicalAddress = (void*)((UPtr)physicalAddress&~0b111111111111);

		const auto directoryIndex = nextLowDirectoryIndex;
		const auto tableIndex = nextLowTableIndex;

		auto &directoryEntry = directory.entry[directoryIndex];

		auto directoryChanged = false;

		// initialising the first table in this directory?
		if(tableIndex==0){
			directoryEntry.isPresent = true;
			directoryEntry.isWritable = false;
			directoryEntry.isUserspaceAccessible = false;

			auto table = (Table*)memory::Transaction().allocate_page()->physicalAddress; //4k alignment needed, so allocate direct as a page
			for(auto i=0;i<1024;i++){
				table->entry[i].isPresent = false;
			}
			directoryEntry.set_table(table);

			directoryChanged = true;
		}

		auto &table = *directoryEntry.get_table();
		auto &tableEntry = table.entry[tableIndex];

		tableEntry.isPresent = true;
		tableEntry.isWritable = options.isWritable;
		tableEntry.isUserspaceAccessible = options.isUserspace;
		tableEntry.pat_writeThroughCaching = options.caching==Caching::writeThrough||options.caching==Caching::writeCombining;
		tableEntry.pat_disableCache = options.caching==Caching::uncached||options.caching==Caching::writeCombining;
		tableEntry.isAccessed = false;
		tableEntry.isDirty = true;
		tableEntry.pat = false;
		tableEntry.isGlobal = false;
		tableEntry.set_address(physicalAddress);

		if(tableEntry.isWritable&&!directoryEntry.isWritable){
			directoryEntry.isWritable = true;
			directoryChanged = true;
		}
		if(tableEntry.isUserspaceAccessible&&!directoryEntry.isUserspaceAccessible){
			directoryEntry.isUserspaceAccessible = true;
			directoryChanged = true;
		}

		//TODO: CLFLUSH?

		{ // invalidates any translation lookaside buffer (TLB) entries for this address

			//NOTE: this only flushes for the current cpu/core
			asm volatile(
				"invlpg [%0]"
				:
				:"m" (physicalAddress)
				:"memory"
			);
		}

		nextLowTableIndex++;
		if(nextLowTableIndex>=1024){
			nextLowTableIndex=0;
			nextLowDirectoryIndex++;

			{ // prep the next directory entry
				auto &directoryEntry = directory.entry[nextLowDirectoryIndex];
				directoryEntry.isPresent = false;
				directoryEntry.isWritable = false;
				directoryEntry.isUserspaceAccessible = false;
				directoryEntry.pat_writeThroughCaching = false;
				directoryEntry.pat_disableCache = false;
				directoryEntry.isAccessed = false;
				directoryEntry.is4Mb = false;

				directoryChanged = true;
			}
		}

		if(directoryChanged){
			//TODO: flush the entire cache with WBINVD (?)
		}

		return (void*)(directoryIndex<<22 | tableIndex<<12 | physicalOffset);
	}

	auto Mapping::map_physical_high(void *physicalAddress, MapOptions options) -> void* {
		// align physicalAddress and store remainder in offset
		auto physicalOffset = (UPtr)physicalAddress&0b111111111111;
		physicalAddress = (void*)((UPtr)physicalAddress&~0b111111111111);

		const auto directoryIndex = nextHighDirectoryIndex;
		const auto tableIndex = nextHighTableIndex;

		auto &directoryEntry = directory.entry[directoryIndex];

		auto directoryChanged = false;

		// initialising the first table in this directory?
		if(tableIndex==1023){
			directoryEntry.isPresent = true;
			directoryEntry.isWritable = false;
			directoryEntry.isUserspaceAccessible = false;

			auto table = (Table*)memory::Transaction().allocate_page()->physicalAddress; //4k alignment needed, so allocate direct as a page
			for(auto i=0;i<1024;i++){
				table->entry[i].isPresent = false;
			}
			directoryEntry.set_table(table);

			directoryChanged = true;
		}

		auto &table = *directoryEntry.get_table();
		auto &tableEntry = table.entry[tableIndex];

		tableEntry.isPresent = true;
		tableEntry.isWritable = options.isWritable;
		tableEntry.isUserspaceAccessible = options.isUserspace;
		tableEntry.pat_writeThroughCaching = options.caching==Caching::writeThrough||options.caching==Caching::writeCombining;
		tableEntry.pat_disableCache = options.caching==Caching::uncached||options.caching==Caching::writeCombining;
		tableEntry.isAccessed = false;
		tableEntry.isDirty = true;
		tableEntry.pat = false;
		tableEntry.isGlobal = false;
		tableEntry.set_address(physicalAddress);

		if(tableEntry.isWritable&&!directoryEntry.isWritable){
			directoryEntry.isWritable = true;
			directoryChanged = true;
		}
		if(tableEntry.isUserspaceAccessible&&!directoryEntry.isUserspaceAccessible){
			directoryEntry.isUserspaceAccessible = true;
			directoryChanged = true;
		}

		//TODO: CLFLUSH?

		{ // invalidates any translation lookaside buffer (TLB) entries for this address

			//NOTE: this only flushes for the current cpu/core
			asm volatile(
				"invlpg [%0]"
				:
				:"m" (physicalAddress)
				:"memory"
			);
		}

		if(nextLowTableIndex==0){
			nextLowTableIndex = 1023;
			nextLowDirectoryIndex--;

			{ // prep the next directory entry
				auto &directoryEntry = directory.entry[nextLowDirectoryIndex];
				directoryEntry.isPresent = false;
				directoryEntry.isWritable = false;
				directoryEntry.isUserspaceAccessible = false;
				directoryEntry.pat_writeThroughCaching = false;
				directoryEntry.pat_disableCache = false;
				directoryEntry.isAccessed = false;
				directoryEntry.is4Mb = false;

				directoryChanged = true;
			}
		}else{
			nextLowTableIndex--;
		}

		if(directoryChanged){
			//TODO: flush the entire cache with WBINVD (?)
		}

		return (void*)(directoryIndex<<22 | tableIndex<<12 | physicalOffset);
	}

	auto Mapping::map_physical_low(void *_address, UPtr size, MapOptions options) -> void* {
		auto address = (U8*)_address;
		auto virtualAddress = map_physical_low(address, options);

		while(size>4096){
			address += 4096;
			size -= 4096;
			map_physical_low(address, options);
		}

		return virtualAddress;
	}

	auto Mapping::map_physical_high(void *_address, UPtr size, MapOptions options) -> void* {
		auto address = (U8*)_address;
		void *virtualAddress;

		address += 4096 * (size/4096);

		while(true) {
			virtualAddress = map_physical_high(address, options);
			if(size<=4096) break;
			address -= 4096;
			size -= 4096;
		}

		return virtualAddress;
	}

	void Mapping::set_virtual_options(void *address, MapOptions options) {
		auto directoryIndex = (UPtr)address>>22;
		auto pageIndex = (UPtr)address>>12 & 0b1111111111;


		auto &tableEntry = directory.entry[directoryIndex].get_table()->entry[pageIndex];

		tableEntry.isUserspaceAccessible = options.isUserspace;
		tableEntry.isWritable = options.isWritable;
		tableEntry.pat_writeThroughCaching = options.caching==Caching::writeThrough||options.caching==Caching::writeCombining;
		tableEntry.pat_disableCache = options.caching==Caching::uncached||options.caching==Caching::writeCombining;

		//TODO: CLFLUSH?

		{ // invalidates any translation lookaside buffer (TLB) entries for this address

			//NOTE: this only flushes for the current cpu/core
			asm volatile(
				"invlpg [%0]"
				:
				:"m" (*(U8*)tableEntry.get_address())
				:"memory"
			);
		}
	}

	void Mapping::set_virtual_options(void *address, UPtr size, MapOptions options) {
		for(auto physicalAddress = (UPtr)address&~0b111111111111; physicalAddress < (UPtr)address+size; physicalAddress += 4096){
			set_virtual_options((void*)physicalAddress, options);
		}
	}
}
