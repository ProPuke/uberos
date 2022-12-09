#include <kernel/mmu.hpp>

#ifdef HAS_MMU

#include <common/format.hpp>
#include <common/stdlib.hpp>

#include <kernel/arch/arm64/systemRegisters.hpp>
#include <kernel/memory.hpp>
#include <kernel/stdio.hpp>

namespace mmu {
	namespace {
		enum struct Granularity {
			_4kb,
			_16kb,
			_64kb
		};

		constexpr const char* granularity_to_string(Granularity granularity){
			switch(granularity){
				case Granularity::_4kb : return "4kb";
				case Granularity::_16kb: return "16kb";
				case Granularity::_64kb: return "64kb";
			}
			return "unknown";
		}
		
		constexpr const char* regionType_to_string(RegionType regionType){
			switch(regionType){
				case RegionType::executable: return "executable";
				case RegionType::memory: return "memory";
				case RegionType::device: return "device";
				case RegionType::deviceMemory: return "deviceMemory";
				case RegionType::test: return "test";
			}
			return "unknown";
		}

		constexpr size_t get_granularity_size(Granularity granularity){
			switch(granularity){
				case Granularity::_4kb : return  4*1024;
				case Granularity::_16kb: return 16*1024;
				case Granularity::_64kb: return 64*1024;
			}
			return 0;
		}

		const auto granularity = Granularity::_16kb;
		static_assert(get_granularity_size(granularity)==memory::pageSize);

		enum struct AddressSize {
			_4GB   = 32,
			_64GB  = 36,
			_1TB   = 40,
			_4TB   = 42,
			_16TB  = 44,
			_256TB = 48,
			_4PB   = 52
		};
		const char *addressSize_to_string(AddressSize size){
			switch(size){
				case AddressSize::_4GB  : return "4GB";
				case AddressSize::_64GB : return "64GB";
				case AddressSize::_1TB  : return "1TB";
				case AddressSize::_4TB  : return "4TB";
				case AddressSize::_16TB : return "16TB";
				case AddressSize::_256TB: return "256TB";
				case AddressSize::_4PB  : return "4PB";
			}
			return "unknown";
		}
		constexpr auto addressSize = AddressSize::_64GB;

		constexpr U64 get_level_bitmask(U8 level);

		// VMSAv8-64 translation tablebzero(initialTable, sizeof(Stage1TableDescriptor)*table0Entries);
		struct Stage1TableDescriptor {
			enum struct AccessPermission {
				kernelReadWrite = 0b00,
				kernelReadOnly  = 0b10,
				readWrite       = 0b01,
				readOnly        = 0b11,
			};

			enum struct Shareable {
				nonShareable   = 0b00,
				unpredictable  = 0b01,
				outerShareable = 0b10,
				innerShareable = 0b11,
			};

			union {
				struct __attribute__((packed)) {
					U64 isValid:1;
					U64 isTable:1;

					// lower attributes
					U64 mairIndex:3;
					U64 nonSecure:1;
					AccessPermission accessPermission:2;
					Shareable shareable:2;
					U64 accessFlag:1;
					U64 nonGlobal:1;

					U64 _addressBits:40;

					// upper block attributes
					U64 block_isContiguous:1;
					U64 block_kernelExecuteNever:1;
					U64 block_executeNever:1;
					U64 block_software:4;

					// next level table attributes
					U64 table_kernelExecuteNever:1;
					U64 table_executeNever:1;
					AccessPermission table_accessPermission:2;
					U64 table_nonSecure:1;
				};

				U64 data;
			};

			void set_unused() {
				isValid = false;
			}

			void set_table(Stage1TableDescriptor *table) {
				isValid = true;
				isTable = true;
				// accessFlag = true;
				// table_nonSecure = true;

				const auto bitmask = ::bitmask(bit_rightmost_position(get_level_bitmask(3)), 47);
				assert(((U64)table&bitmask)==(U64)table, "table ", table, " does not align with bitmask ", format::Hex64{bitmask});
				data = data&~bitmask | (U64)table&bitmask;

				// stdio::print_debug("table ", this, " ", format::Hex64{data});
			}

			void set_block(U8 level, void *address) {
				isValid = true;
				isTable = level==3;
				accessFlag = true;

				const auto bitmask = ::bitmask(bit_rightmost_position(get_level_bitmask(level)), 47);
				assert(((U64)address&bitmask)==(U64)address, "block ", address, " does not align with bitmask ", format::Hex64{bitmask});
				data = data&~bitmask | (U64)address&bitmask;

				// stdio::print_debug("block ", this, " ", format::Hex64{data});
			}

			constexpr auto get_table_address() const {
				const auto bitmask = ::bitmask(bit_rightmost_position(get_level_bitmask(3)), 47);
				return (Stage1TableDescriptor*)(data&bitmask);
			}

			constexpr auto get_block_address(U8 level) const {
				const auto bitmask = ::bitmask(bit_rightmost_position(get_level_bitmask(level)), 47);
				return (void*)(data & bitmask);
			}
		};
		static_assert(sizeof(Stage1TableDescriptor)==8);

		constexpr U64 get_level_bitmask(U8 level){
			constexpr U64 mask = bitmask(0, (U8)addressSize-1);

			switch(granularity){
				case Granularity::_4kb:
					switch(level){
						case 0: return mask & 0b00000000'00000000'11111111'10000000'00000000'00000000'00000000'00000000; // 512GB
						case 1: return mask & 0b00000000'00000000'00000000'01111111'11000000'00000000'00000000'00000000; // 1GB
						case 2: return mask & 0b00000000'00000000'00000000'00000000'00111111'11100000'00000000'00000000; // 2MB
						case 3: return mask & 0b00000000'00000000'00000000'00000000'00000000'00011111'11110000'00000000; // 4KB
						default: return 0;
					}
				break;
				case Granularity::_16kb:
					switch(level){
						case 0: return mask & 0b00000000'00000000'10000000'00000000'00000000'00000000'00000000'00000000; // 128TB
						case 1: return mask & 0b00000000'00000000'01111111'11110000'00000000'00000000'00000000'00000000; // 64GB
						case 2: return mask & 0b00000000'00000000'00000000'00001111'11111110'00000000'00000000'00000000; // 32MB
						case 3: return mask & 0b00000000'00000000'00000000'00000000'00000001'11111111'11000000'00000000; // 16KB
						default: return 0;
					}
				break;
				case Granularity::_64kb:
					switch(level){
						case 0: return mask & 0b0;
						case 1: return mask & 0b00000000'00001111'11111100'00000000'00000000'00000000'00000000'00000000; // 4TB
						case 2: return mask & 0b00000000'00000000'00000011'11111111'11100000'00000000'00000000'00000000; // 512MB
						case 3: return mask & 0b00000000'00000000'00000000'00000000'00011111'11111111'00000000'00000000; // 64KB
						default: return 0;
					}
				break;
			}

			return 0;
		}

		// hardware translation table sizes
		constexpr unsigned levelSize[4] = {
			1<<bit_count(get_level_bitmask(0)),
			1<<bit_count(get_level_bitmask(1)),
			1<<bit_count(get_level_bitmask(2)),
			1<<bit_count(get_level_bitmask(3)),
		};

		// what's the first translation level? taking into account skipped initial tables
		constexpr unsigned startLevel = levelSize[0]>1?0:levelSize[1]>1?1:2;
	}

	struct TableDescriptor:Stage1TableDescriptor{};

	MemoryMapping kernelMapping(false);

	void init_kernelMap();

	void init() {
		stdio::Section section("mmu::arch::arm64::init...");

		stdio::print_info("granularity: ", granularity_to_string(granularity));
		stdio::print_info("address size: ", (int)addressSize, "bit (", addressSize_to_string(addressSize), ")");
		stdio::print_info("table sizes: ", levelSize[0], " / ", levelSize[1], " / ", levelSize[2], " / ", levelSize[3]);

		kernelMapping.init();

		MemoryMapping mapping;

		init_kernelMap();

		{
			// void *address = (void*)0x82373584;
			// void *address = (void*)0x85a00;
			void *address = (void*)0x3686C8;

			U64 level0 = ((U64)address&get_level_bitmask(0))>>bit_rightmost_position(get_level_bitmask(0));
			U64 level1 = ((U64)address&get_level_bitmask(1))>>bit_rightmost_position(get_level_bitmask(1));
			U64 level2 = ((U64)address&get_level_bitmask(2))>>bit_rightmost_position(get_level_bitmask(2));
			U64 level3 = ((U64)address&get_level_bitmask(3))>>bit_rightmost_position(get_level_bitmask(3));
			U64 offset = bits((U64)address,0,bit_rightmost_position(get_level_bitmask(3))-1);

			stdio::print_debug("address = ", address);

			stdio::print_debug("level0 = ", level0);
			stdio::print_debug("level1 = ", level1);
			stdio::print_debug("level2 = ", level2);
			stdio::print_debug("level3 = ", level3);
			stdio::print_debug("offset = ", offset);

			const auto &entry2 = kernelMapping.initialTable[level2];
			stdio::print_debug("table2 = ", format::Hex64{entry2.data});

			if(entry2.isTable){
				const auto &entry3 = entry2.get_table_address()[level3];
				stdio::print_debug("table3 = ", format::Hex64{entry3.data});

				auto page = entry3.get_block_address(3);
				stdio::print_debug("page   = ", (U64)page);
				stdio::print_debug("destination = ", (void*)((U64)page+offset));
				stdio::print_debug("address     = ", address);

			}else{
				auto page = entry2.get_block_address(2);
				stdio::print_debug("page   = ", (U64)page);
				stdio::print_debug("destination = ", (void*)((U64)page+((U64)address&get_level_bitmask(3))|offset));
				stdio::print_debug("address     = ", address);
			}

			// while(true);
		}

		set_kernelspace_mapping(kernelMapping);
		set_userspace_mapping(kernelMapping);

		enable();
	}
		
	void enable() {
		stdio::Section section("mmu::arch::arm64::enable...");

		U8 tg0GranuleSize; // NOTE: these two don't match in format cos arm is weird
		U8 tg1GranuleSize;
		switch(granularity){
			case Granularity::_4kb:
				tg0GranuleSize = 0b00;
				tg1GranuleSize = 0b10;
			break;
			case Granularity::_16kb:
				tg0GranuleSize = 0b10;
				tg1GranuleSize = 0b01;
			break;
			case Granularity::_64kb:
				tg0GranuleSize = 0b11;
				tg1GranuleSize = 0b11;
			break;
		}

		U8 ipsBits;
		switch(addressSize){
			case AddressSize::_4GB:
				ipsBits = 0b000;
			break;
			case AddressSize::_64GB:
				ipsBits = 0b001;
			break;
			case AddressSize::_1TB:
				ipsBits = 0b010;
			break;
			case AddressSize::_4TB:
				ipsBits = 0b011;
			break;
			case AddressSize::_16TB:
				ipsBits = 0b100;
			break;
			case AddressSize::_256TB:
				ipsBits = 0b101;
			break;
			case AddressSize::_4PB:
				ipsBits = 0b110;
			break;
		}

		{
			stdio::Section section("tcr_el1");

			Tcr tcr_el1;
			tcr_el1.load_el1();

			tcr_el1.t0sz = 64-(int)addressSize;
			tcr_el1.t1sz = 64-(int)addressSize;

			tcr_el1.ips = ipsBits;

			tcr_el1.tbi0 = 0; //do not ignore first bit of ttbr0 address
			tcr_el1.tbi1 = 0; //do not ignore first bit of ttbr1 address

			tcr_el1.tg0 = tg0GranuleSize;
			tcr_el1.tg1 = tg1GranuleSize;

			tcr_el1.irgn0 = 0b11; //normal memory, outer write-back read-allocate no write-allocate cacheable
			tcr_el1.irgn1 = 0b11; //normal memory, outer write-back read-allocate no write-allocate cacheable

			tcr_el1.orgn0 = 0b11; //normal memory, outer write-back read-allocate no write-allocate cacheable
			tcr_el1.orgn1 = 0b11; //normal memory, outer write-back read-allocate no write-allocate cacheable

			tcr_el1.sh0 = 0b11; // inner shareable
			tcr_el1.sh1 = 0b11; // inner shareable

			tcr_el1.save_el1();
		}

		{
			stdio::Section section("mair_el1");

			Mair mair_el1;
			mair_el1.load_el1();

			mair_el1.entry[0] = 0xff; //NOTE:HACK: "normal" memory
			mair_el1.entry[1] = 0x44; //NOTE:HACK: "non-cache" memory?
			mair_el1.entry[2] = 0x00; //NOTE:HACK: NGNRNE whatever the hell that means

			mair_el1.save_el1();
		}

		{
			stdio::Section section("sctlr_el1");

			Sctlr sctlr_el1;
			sctlr_el1.load_el1();

			sctlr_el1.mmuEnable = true;
			// sctlr_el1.alignmentChecking = true;
			sctlr_el1.cacheEnable = true;
			// sctlr_el1.stackAlignmentCheck = true;
			// sctlr_el1.stackAlignmentCheckEl0 = true;
			sctlr_el1.instructionCache = true;
			sctlr_el1.exceptionExitIsContextSwitching = true;
			sctlr_el1.exceptionEntryIsContextSwitching = true;
			// sctlr_el1.setPrivilegedAccessNeverOnEl1 = true;

			sctlr_el1.save_el1();
		}

		_is_enabled = true;
	}

	void disable() {
		stdio::Section section("mmu::arch::arm64::disable...");

		{
			stdio::Section section("sctlr_el1");

			Sctlr sctlr_el1;
			sctlr_el1.load_el1();

			sctlr_el1.mmuEnable = false;

			sctlr_el1.save_el1();
		}

		_is_enabled = false;
	}

	void set_kernelspace_mapping(MemoryMapping &memoryMapping) {
		Ttbr ttbr1_el1;
		ttbr1_el1.load_br1el1();

		U8 lowBit;
		switch(granularity){
			case Granularity::_4kb:
				lowBit = 12;
			break;
			case Granularity::_16kb:
				lowBit = 14;
			break;
			case Granularity::_64kb:
				lowBit = 16;
			break;
		}

		ttbr1_el1.set_tableAddress(lowBit, memoryMapping.initialTable);
		ttbr1_el1.commonNotPrivate = false;

		ttbr1_el1.save_br1el1();
	}

	void set_userspace_mapping(MemoryMapping &memoryMapping) {
		Ttbr ttbr0_el1;
		ttbr0_el1.load_br0el1();

		U8 lowBit;
		switch(granularity){
			case Granularity::_4kb:
				lowBit = 12;
			break;
			case Granularity::_16kb:
				lowBit = 14;
			break;
			case Granularity::_64kb:
				lowBit = 16;
			break;
		}

		ttbr0_el1.set_tableAddress(lowBit, memoryMapping.initialTable);
		ttbr0_el1.commonNotPrivate = false;

		ttbr0_el1.save_br0el1();
	}

	/**/ MemoryMapping::MemoryMapping(bool doInit){
		if(doInit){
			init();
		}
	}

	/**/ MemoryMapping::~MemoryMapping(){
		if(initialTable){
			clear();
			memory::Transaction().free_page_with_address(initialTable);
		}
	}

	void MemoryMapping::init() {
		const auto requiredSize = levelSize[startLevel]*sizeof(TableDescriptor);
		const auto requiredPages = (requiredSize+memory::pageSize-1)/memory::pageSize;
		assert(requiredPages==1, "mmu table does not sit in a single page"); // we only free a single page, so make sure we only allocate 1, too
		auto page = memory::Transaction().allocate_pages(requiredPages);
		auto address = page->physicalAddress;
		initialTable = (TableDescriptor*)address;
		bzero(initialTable, requiredSize);
	}

	namespace {
		inline void _clear(Stage1TableDescriptor *table, U8 level, memory::Transaction &transaction){
			for(unsigned i=0;i<levelSize[level];i++){
				auto &entry = table[i];
				if(!entry.isValid) break;

				if(entry.isTable){
					auto subTable = entry.get_table_address();
					_clear(subTable, level+1, transaction);
					transaction.free_page_with_address(subTable);

				}else{
					auto address = entry.get_block_address(level);
					transaction.free_page_with_address(address);
				}

				entry.set_unused();
			}
		}

		// if we wanted to be most efficient with translation tables we would always allocate blocks in the largest tables possible
		// we could use the software bits to mark when nested levels in the tables were full and accelerate searches
		// however, that would skip addresses and return non-sequential address, so for now we'll do the dumb thing and allocate
		// in sequence in each time. We'll be wasting a lot of tables (for now), but addresses will always increase sequentially
		// we can look at defragmenting the physical addresses later, so that tables can be swapped out for larger chunks
		// if this becomes a problem

		inline void* _insert(Stage1TableDescriptor *table, unsigned level, void *&address, unsigned &pages, RegionType regionType, memory::Transaction &transaction){
			// stdio::Section section("insert ", pages, ' ', regionType_to_string(regionType), " pages into level ", level, "...");

			void *baseVirtualAddress = nullptr;
			bool firstAddress = true;

			for(U64 i=0;i<levelSize[level];i++){
				auto &entry = table[i];

				if(!entry.isValid){
					// stdio::print_debug("entry ", i, " is free...");

					//empty slot
					const U64 entrySize = level+1>3?1:(U64)levelSize[level+1]*(level+2>3?1:(U64)levelSize[level+2]*(level+3>3?1:(U64)levelSize[level+3]));
					if(pages>=entrySize){
						//allocate a whole block
						if(firstAddress){
							firstAddress = false;
							baseVirtualAddress = (void*)(i*bit_rightmost(get_level_bitmask(level)));
							// stdio::print_debug("got base address of ", baseVirtualAddress);
						}
						// stdio::print_debug("insert ", pages, " pages into ", entrySize, " slot");
						entry.set_block(level, address);
						entry.accessPermission = Stage1TableDescriptor::AccessPermission::kernelReadWrite;
						switch(regionType){
							case RegionType::executable:
								entry.mairIndex = 0;
								entry.shareable = Stage1TableDescriptor::Shareable::innerShareable;
								entry.block_kernelExecuteNever = false;
								entry.block_executeNever = false;
							break;
							case RegionType::memory:
								entry.mairIndex = 0;
								entry.shareable = Stage1TableDescriptor::Shareable::innerShareable;
								entry.block_kernelExecuteNever = true;
								entry.block_executeNever = true;
							break;
							case RegionType::deviceMemory:
								entry.mairIndex = 1;
								entry.shareable = Stage1TableDescriptor::Shareable::nonShareable;
								entry.block_kernelExecuteNever = true;
								entry.block_executeNever = true;
							break;
							case RegionType::device:
								entry.mairIndex = 2;
								entry.shareable = Stage1TableDescriptor::Shareable::nonShareable;
								entry.block_kernelExecuteNever = true;
								entry.block_executeNever = true;
							break;
							case RegionType::test:
								entry.mairIndex = 2;
								entry.shareable = Stage1TableDescriptor::Shareable::innerShareable;
								entry.block_kernelExecuteNever = false;
								entry.block_executeNever = false;
							break;
						}
						// stdio::print_debug("block ", &entry, " ", format::Hex64{entry.data});
						// stdio::print_debug("allocating block of ", entrySize, "...");
						pages -= entrySize;
						address = (void*)((U64)address+entrySize*memory::pageSize);
						// stdio::print_debug("address is now ", address);
						if(!pages) return baseVirtualAddress;
						continue;

					}else{
						// stdio::print_debug("too big (", entrySize, ')');
						// stdio::print_debug("allocating a subtable...");

						//allocate a subtable
						const auto requiredSize = levelSize[level+1]*sizeof(TableDescriptor);
						const auto requiredPages = (requiredSize+memory::pageSize-1)/memory::pageSize;
						assert(requiredPages==1, "mmu table does not sit in a single page"); // we only free a single page, so make sure we only allocate 1, too
						auto subtable = (Stage1TableDescriptor*)transaction.allocate_pages(requiredPages)->physicalAddress;
						bzero(subtable, requiredSize);
						entry.set_table(subtable);
					}

				}else if(!entry.isTable||level>=3){
					// stdio::print_debug("skipping existing block...");

					//existing block
					continue;

				}else{
					//existing table
					// stdio::print_debug("existing table...");

					// if(i+1<levelSize[level]&&table[i+1].isValid) stdio::print_debug("skipping existing table, as there's a next one...");
					if(i+1<levelSize[level]&&table[i+1].isValid) continue; //jump forward if there is a next valid item ahead

					auto subtable = entry.get_table_address();
					// if(subtable[levelSize[level+1]-1].isValid) stdio::print_debug("skipping existing table, as its child slots are all full...");
					if(subtable[levelSize[level+1]-1].isValid) continue; //jump forward if subtable is filled

					// stdio::print_debug("inserting into existing table...");
				}

				auto virtualAddress = _insert(entry.get_table_address(), level+1, address, pages, regionType, transaction);
				if(firstAddress){
					firstAddress = false;
					baseVirtualAddress = (void*)(i*bit_rightmost(get_level_bitmask(level))|(U64)virtualAddress);
					// stdio::print_debug("got base address of ", baseVirtualAddress);
				}

				// if(!pages) stdio::print_debug("all pages added");
				if(!pages) return baseVirtualAddress;
			}

			return baseVirtualAddress;
		}
	}

	void MemoryMapping::clear() {
		const auto initialTable = this->initialTable;

		memory::Transaction memoryTransaction;
		_clear(initialTable, startLevel, memoryTransaction);

		pageCount = 0;
	}

	void* MemoryMapping::add_pages(U32 count, RegionType regionType) {
		if(count<1) return nullptr;

		memory::Transaction memoryTransaction;

		const auto pages = memoryTransaction.allocate_pages(count);
		if(!pages) return nullptr;

		const auto initialTable = this->initialTable;

		void *virtualBase = nullptr;

		// insert the pages in chunks of consecutive memory (to assist in simplifying tables)
		for(unsigned i=0;i<count;){
			unsigned i2;
			for(i2=i+1;i2<count;i2++){
				if(pages[i2].physicalAddress!=(U8*)pages[i2-1].physicalAddress+memory::pageSize){
					break;
				}
			}

			unsigned consecutivePages = i2-i;

			//TODO: this should take chunks, not pages
			// should probably lock granularity to match pagesize to make all this simpler

			auto pagesToInsert = consecutivePages;
			auto virtualAddress = _insert(initialTable, startLevel, pages[i].physicalAddress, pagesToInsert, regionType, memoryTransaction);

			if(pagesToInsert>0){
				//OH NO! Couldn't fit it all into the tables
				//will just freak out and fail for now, with the tables now entirely filled :S
				return nullptr;
			}

			if(i==0){
				virtualBase = virtualAddress;
			}

			i += consecutivePages;
		}

		return virtualBase;
	}

	void* MemoryMapping::add_mapping(void *address, U32 pages, RegionType regionType) {
		if(pages<1) return nullptr;

		memory::Transaction memoryTransaction;
		const auto initialTable = this->initialTable;

		return _insert(initialTable, startLevel, address, pages, regionType, memoryTransaction);
	}

	void* MemoryMapping::add_mapping(void *addressStart, void *addressEnd, RegionType regionType) {
		stdio::print_debug("map ", addressStart, " -> ", addressEnd, " as ", regionType_to_string(regionType));

		const auto pageCount = ((U64)addressEnd-(U64)addressStart+memory::pageSize-1)/memory::pageSize;
		auto virtualAddress = add_mapping(addressStart, pageCount, regionType);
		// stdio::print_debug("  = ", virtualAddress);

		return virtualAddress;
	}
}

#endif
