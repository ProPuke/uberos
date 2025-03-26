#include "mmu.hpp"

#include <kernel/arch/x86/cpuInfo.hpp>
#include <kernel/assert.hpp>
#include <kernel/memory.hpp>

namespace mmu {
	Mapping kernelMapping __attribute__((aligned(4096))); //NOTE:HACK: This assumes `directory` is the first member. Really that member should have this alignment enforcement?

	namespace {
		Physical<Mapping::Table> tablesPhysical;
		Mapping::Table *tables = 0;
		auto nextTableIndex = 0u;
		UPtr tablesCount = 0;

		auto to_virtual(Physical<Mapping::Table> table) -> Mapping::Table& {
			debug::assert(table.address>=tablesPhysical.address); // is within bounds
			auto offset = table.address-tablesPhysical.address;
			debug::assert(offset%sizeof(Mapping::Table) == 0); // is aligned

			auto index = offset/sizeof(Mapping::Table);
			debug::assert(index<tablesCount); // is within bounds

			return tables[index];
		}

		auto to_physical(Mapping::Table &table) -> Physical<Mapping::Table> {
			debug::assert((UPtr)&table>=(UPtr)tables); // is within bounds
			debug::assert((UPtr)&table<(UPtr)&tables[tablesCount]); // is within bounds

			auto offset = (UPtr)&table-(UPtr)tables;
			debug::assert(offset%sizeof(Mapping::Table) == 0); // is aligned

			return tablesPhysical+offset;
		}

		auto create_table() -> Mapping::Table& {
			assert(nextTableIndex<tablesCount);
			//TODO: fail gracefully if out of tables
			return tables[nextTableIndex++];
		}
	}

	void init() {
		assert(::arch::x86::cpuInfo::get_features().pat, "PAT not supported by cpu");

		{
			// allocate enough to address all heap (plus 1/8th for shared memory overlap)
			auto pages = (((memory::heapSize+memory::pageSize-1)/memory::pageSize)*9/8 / 1024 * sizeof(Mapping::Table) + memory::pageSize-1) / memory::pageSize;
			auto tablesSize = pages * memory::pageSize;

			assert(memory::heapSize>=tablesSize);
			tablesPhysical = memory::heap.as_type<mmu::Mapping::Table>();
			memory::heap += tablesSize;
			memory::heapSize -= tablesSize;

			tablesCount = tablesSize / sizeof(Mapping::Table);

			tables = (Mapping::Table*)tablesPhysical.address; // tables are 1:1 mapped initially, until activated
		}

		static Mapping::Table initialTable;

		kernelMapping.nextFreeHighTable = &initialTable;

		{
			auto kernelTransaction = kernel::transaction();

			// we'll identity map the stack as low. This is fine - we won't need the stack while running from other mappings
			kernelTransaction.map_physical_low(Physical<void>{0x00}, memory::stack.address+memory::stackSize, {.caching = Caching::uncached});

			// kernelTransaction.map_physical_low(Physical<void>{0x00}, 1024*4096, {.caching = Caching::uncached});
			
			kernelTransaction.set_virtual_options((void*)memory::code.address, memory::codeSize, {.isWritable = false, .isExecutable = true});

			// identity mapped, so using memory::stack.address direct is safe
			kernelTransaction.set_virtual_options((void*)memory::stack.address, memory::stackSize, {});

			// ensure tables are accessible once active..
			tables = (Mapping::Table*) kernelTransaction.map_physical_high(tablesPhysical, tablesCount*sizeof(Mapping::Table), {});
		}

		// ..and finally turn it on!
		activate(kernelMapping);
	}

	void activate(Mapping &mapping) {
		asm volatile(R"(
			// set directory
			mov cr3, eax

			// activate
			// mov eax, cr0
			// or eax, 0x80000000
			// mov cr0, eax

			mov cr0, ebx
		)"
			:
			:
				// "r"(mapping.get_physical(&mapping.directory).address)
				"a"(&mapping.directory), // identity mapped
				"b"(0
					|1<<0  // PE - protected mode enable
					|1<<5  // NE - numeric error
					|1<<16 // WP - write protect
					|1<<31 // PG - paging enable
				)
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

	namespace kernel {
		void _lock() { return kernelMapping._lock(); }
		void _unlock() { return kernelMapping._unlock(); }

		auto _map_physical_low(Physical<void> physical, MapOptions options) -> void* {
			#ifdef KERNEL_MMU
				return kernelMapping._map_physical_low(physical, options);
			#else
				return (void*)physical.address;
			#endif
		}
		auto _map_physical_low(Physical<void> physical, UPtr size, MapOptions options) -> void* {
			#ifdef KERNEL_MMU
				return kernelMapping._map_physical_low(physical, size, options);
			#else
				return (void*)physical.address;
			#endif
		}
		auto _map_physical_high(Physical<void> physical, MapOptions options) -> void* {
			#ifdef KERNEL_MMU
				return kernelMapping._map_physical_high(physical, options);
			#else
				return (void*)physical.address;
			#endif
		}
		auto _map_physical_high(Physical<void> physical, UPtr size, MapOptions options) -> void* {
			#ifdef KERNEL_MMU
				return kernelMapping._map_physical_high(physical, size, options);
			#else
				return (void*)physical.address;
			#endif
		}

		void _set_virtual_target(void *address, Physical<void> physicalAddress, UPtr size) {
			#ifdef KERNEL_MMU
				kernelMapping._set_virtual_target(address, physicalAddress, size);
			#else
				assert(false); // virtual addresses cannot be pointed elsewhere without mmu
			#endif
		}

		void _set_virtual_options(void *address, MapOptions options) {
			#ifdef KERNEL_MMU
				kernelMapping._set_virtual_options(address, options);
			#endif
		}
		void _set_virtual_options(void *address, UPtr size, MapOptions options) {
			#ifdef KERNEL_MMU
				kernelMapping._set_virtual_options(address, size, options);
			#endif
		}
		auto _get_virtual_options(void *address) -> Try<MapOptions> {
			#ifdef KERNEL_MMU
				return kernelMapping._get_virtual_options(address);
			#endif

			return "Virtual memory not enabled";
		}

		auto _get_physical(void *virtualAddress) -> Physical<void> {
			#ifdef KERNEL_MMU
				return kernelMapping._get_physical(virtualAddress);
			#else
				return Physical<void>{(UPtr)virtualAddress};
			#endif
		}
	}

	/**/ Mapping:: Mapping() {
		for(auto i=0u;i<1024;i++){
			directory.entry[i].isPresent = false;
		}
	}

	/**/ Mapping::~Mapping() {
		// clear();
	}

	// void Mapping::clear() {
	// 	// for(auto directoryIndex=0;directoryIndex<=nextLowDirectoryIndex;directoryIndex++){
	// 	// 	auto &directoryEntry = directory.entry[directoryIndex];
	// 	// 	if(directoryEntry.isPresent){
	// 	// 		memory::Transaction().free_page_with_address(directoryEntry.get_table());
	// 	// 		directoryEntry.isPresent = false;
	// 	// 	}
	// 	// }

	// 	// nextLowDirectoryIndex = 0;
	// }

	void Mapping::_lock() {
		lock.lock();
	}

	void Mapping::_unlock() {
		lock.unlock();
	}

	auto Mapping::_map_physical_low(Physical<void> _physical, MapOptions options) -> void* {
		debug::assert(_physical.address<=0xffffffff);
		auto physical = Physical32<void>{_physical.address};

		// align physicalAddress and store remainder in offset
		auto physicalOffset = physical.address&0b111111111111;
		physical.address = physical.address&~0b111111111111;

		const auto directoryIndex = nextLowDirectoryIndex;
		const auto tableIndex = nextLowTableIndex;

		auto virtualAddress = (void*)(directoryIndex<<22 | tableIndex<<12 | physicalOffset);

		auto &directoryEntry = directory.entry[directoryIndex];

		auto directoryChanged = false;

		const auto isFirstTable = tableIndex==0;

		// initialising the first table in this directory?
		if(isFirstTable){
			debug::assert(!directoryEntry.isPresent);
			directoryEntry.isPresent = true;
			debug::assert(directoryEntry.isPresent);
			directoryEntry.isWritable = false;
			directoryEntry.isUserspaceAccessible = false;
			directoryEntry.pat_writeThroughCaching = false;
			directoryEntry.pat_disableCache = false;
			directoryEntry.isAccessed = false;
			directoryEntry.isDirty = false;
			directoryEntry.is4Mb = false;

			auto &nextTable = create_table();
			for(auto i=0;i<1024;i++){
				nextTable.entry[i].isPresent = false;
			}
			directoryEntry.set_table(to_physical(nextTable));

			directoryChanged = true;
		}

		auto &table = to_virtual(directoryEntry.get_table());
		auto &tableEntry = table.entry[tableIndex];
		debug::assert(!tableEntry.isPresent);

		tableEntry.isPresent = true;
		debug::assert(tableEntry.isPresent);
		tableEntry.isWritable = options.isWritable;
		tableEntry.isUserspaceAccessible = options.isUserspace;
		tableEntry.pat_writeThroughCaching = options.caching==Caching::writeThrough||options.caching==Caching::writeCombining;
		tableEntry.pat_disableCache = options.caching==Caching::uncached||options.caching==Caching::writeCombining;
		tableEntry.isAccessed = false;
		tableEntry.isDirty = true;
		tableEntry.pat = false;
		tableEntry.isGlobal = false;
		tableEntry.set_address(physical);

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
			//TODO: CHECK: Do we need to do this if this mapping isn't active? if not, do we need to do something special when we do switch to it?
			asm volatile(
				"invlpg %0"
				:
				:"m" (*(UPtr*)virtualAddress)
				:"memory"
			);
		}

		if(nextLowTableIndex==1023){
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
		}else{
			nextLowTableIndex++;
		}

		if(directoryChanged){
			//TODO: flush the entire cache with WBINVD (?)
		}

		return virtualAddress;
	}

	auto Mapping::_map_physical_high(Physical<void> _physical, MapOptions options) -> void* {
		debug::assert(_physical.address<=0xffffffff);
		auto physical = Physical32<void>{_physical.address};

		// align physicalAddress and store remainder in offset
		auto physicalOffset = physical.address&0b111111111111;
		physical.address = physical.address&~0b111111111111;

		const auto directoryIndex = nextHighDirectoryIndex;
		const auto tableIndex = nextHighTableIndex;

		auto virtualAddress = (void*)(directoryIndex<<22 | tableIndex<<12 | physicalOffset);

		auto &directoryEntry = directory.entry[directoryIndex];

		auto directoryChanged = false;

		const auto isFirstTable = tableIndex==1023||directoryIndex==1023&&tableIndex==1022; // we skip the final table when initialising the last directory

		// initialising the first table in this directory?
		if(isFirstTable){
			debug::assert(!directoryEntry.isPresent);
			directoryEntry.isPresent = true;
			debug::assert(directoryEntry.isPresent);
			directoryEntry.isWritable = false;
			directoryEntry.isUserspaceAccessible = false;
			directoryEntry.pat_writeThroughCaching = false;
			directoryEntry.pat_disableCache = false;
			directoryEntry.isAccessed = false;
			directoryEntry.isDirty = false;
			directoryEntry.is4Mb = false;

			auto &nextTable = create_table();

			for(auto i=0;i<1024;i++){
				nextTable.entry[i].isPresent = false;
			}
			directoryEntry.set_table(to_physical(nextTable));

			directoryChanged = true;
		}

		auto &table = to_virtual(directoryEntry.get_table());
		auto &tableEntry = table.entry[tableIndex];

		tableEntry.isPresent = true;
		debug::assert(tableEntry.isPresent);
		tableEntry.isWritable = options.isWritable;
		tableEntry.isUserspaceAccessible = options.isUserspace;
		tableEntry.pat_writeThroughCaching = options.caching==Caching::writeThrough||options.caching==Caching::writeCombining;
		tableEntry.pat_disableCache = options.caching==Caching::uncached||options.caching==Caching::writeCombining;
		tableEntry.isAccessed = false;
		tableEntry.isDirty = true;
		tableEntry.pat = false;
		tableEntry.isGlobal = false;
		tableEntry.set_address(physical);

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
			//TODO: CHECK: Do we need to do this if this mapping isn't active? if not, do we need to do something special when we do switch to it?
			asm volatile(
				"invlpg %0"
				:
				:"m" (*(UPtr*)virtualAddress)
				:"memory"
			);
		}

		if(nextHighTableIndex==0){
			nextHighTableIndex = 1023;
			nextHighDirectoryIndex--;

			{ // prep the next directory entry
				auto &directoryEntry = directory.entry[nextHighDirectoryIndex];
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
			nextHighTableIndex--;
		}

		if(directoryChanged){
			//TODO: flush the entire cache with WBINVD (?)
		}

		return virtualAddress;
	}

	auto Mapping::_map_physical_low(Physical<void> physical, UPtr size, MapOptions options) -> void* {
		auto virtualAddress = _map_physical_low(physical, options);

		while(size>4096){
			physical += 4096;
			size -= 4096;
			_map_physical_low(physical, options);
		}

		return virtualAddress;
	}

	auto Mapping::_map_physical_high(Physical<void> physical, UPtr size, MapOptions options) -> void* {
		if(size<1) size = 1;

		auto pages = ((size+4095)/4096);
		physical += (pages-1) * 4096;

		void *virtualAddress = nullptr;
		for(;pages>0;pages--){
			virtualAddress = _map_physical_high(physical, options);
			physical.address -= 4096;
		}

		return virtualAddress;
	}

	// auto Mapping::unmap_virtual_high(void *virtualAddress, UPtr size) -> void* {
	// 	auto directoryIndex = (UPtr)virtualAddress>>22 & 0x3ff;
	// 	auto tableIndex = (UPtr)virtualAddress>>12 & 0x3ff;
	// 	auto physicalOffset = (UPtr)virtualAddress&0xfff;

	// 	auto &directoryEntry = directory.entry[directoryIndex];
	// 	auto &table = to_virtual(directoryEntry.get_table());
	// 	auto &tableEntry = table.entry[tableIndex];

	// 	table
	// }

	void Mapping::_set_virtual_target(void *address, Physical<void> physicalAddress, UPtr size) {
		for(auto virtualAddress = (UPtr)address&~0b111111111111; virtualAddress < (UPtr)address+size; virtualAddress += 4096, physicalAddress += 4096){
			auto directoryIndex = (UPtr)virtualAddress>>22 & 0x3ff;
			auto pageIndex = (UPtr)virtualAddress>>12 & 0x3ff;

			auto &tableEntry = to_virtual(directory.entry[directoryIndex].get_table()).entry[pageIndex];

			tableEntry.set_address(physicalAddress.as_size<U32>());

			asm volatile(
				"invlpg %0"
				:
				:"m" (*(UPtr*)virtualAddress)
				:"memory"
			);
		}
	}

	void Mapping::_set_virtual_options(void *virtualAddress, MapOptions options) {
		auto directoryIndex = (UPtr)virtualAddress>>22 & 0x3ff;
		auto pageIndex = (UPtr)virtualAddress>>12 & 0x3ff;

		auto &tableEntry = to_virtual(directory.entry[directoryIndex].get_table()).entry[pageIndex];

		tableEntry.isUserspaceAccessible = options.isUserspace;
		tableEntry.isWritable = options.isWritable;
		tableEntry.pat_writeThroughCaching = options.caching==Caching::writeThrough||options.caching==Caching::writeCombining;
		tableEntry.pat_disableCache = options.caching==Caching::uncached||options.caching==Caching::writeCombining;

		//TODO: CLFLUSH?

		{ // invalidates any translation lookaside buffer (TLB) entries for this address

			//NOTE: this only flushes for the current cpu/core
			//TODO: CHECK: Do we need to do this if this mapping isn't active? if not, do we need to do something special when we do switch to it?
			asm volatile(
				"invlpg %0"
				:
				:"m" (*(UPtr*)virtualAddress)
				:"memory"
			);
		}
	}

	void Mapping::_set_virtual_options(void *address, UPtr size, MapOptions options) {
		for(auto virtualAddress = (UPtr)address&~0b111111111111; virtualAddress < (UPtr)address+size; virtualAddress += 4096){
			_set_virtual_options((void*)virtualAddress, options);
		}
	}

	auto Mapping::_get_virtual_options(void *virtualAddress) -> Try<MapOptions> {
		auto directoryIndex = (UPtr)virtualAddress>>22 & 0x3ff;
		auto pageIndex = (UPtr)virtualAddress>>12 & 0x3ff;

		auto table = directory.entry[directoryIndex].get_table();
		if(!table) return {"Page does not exist"};

		auto &tableEntry = to_virtual(table).entry[pageIndex];

		MapOptions options;
		options.isUserspace = tableEntry.isUserspaceAccessible;
		options.isWritable = tableEntry.isWritable;
		options.isExecutable = true;

		if(tableEntry.pat_writeThroughCaching){
			if(tableEntry.pat_disableCache){
				options.caching = Caching::writeCombining;
			}else{
				options.caching = Caching::writeThrough;
			}
		}else{
			if(tableEntry.pat_disableCache){
				options.caching = Caching::uncached;
			}else{
				options.caching = Caching::writeBack;
			}
		}

		return options;
	}

	auto Mapping::_get_physical(void *virtualAddress) -> Physical<void> {
		auto directoryIndex = (UPtr)virtualAddress>>22 & 0x3ff;
		auto tableIndex = (UPtr)virtualAddress>>12 & 0x3ff;
		auto physicalOffset = (UPtr)virtualAddress&0xfff;

		auto &directoryEntry = directory.entry[directoryIndex];
		auto &table = to_virtual(directoryEntry.get_table());
		auto &tableEntry = table.entry[tableIndex];

		return tableEntry.get_address().as_native() + physicalOffset;
	}
}
