#include "mmu.hpp"

#include <common/stdlib.hpp>

#include <kernel/arch/raspi/mailbox.hpp>
#include <kernel/memory.hpp>
#include <kernel/stdio.hpp>
#include <kernel/arch/arm64/systemRegisters.hpp>

namespace mailbox {
	using namespace arch::raspi::mailbox;
}

namespace mmio {
	using namespace arch::raspi;
}

namespace mmu {
	namespace arch {
		namespace arm64 {
			namespace {
				enum struct Granularity {
					_4kb,
					_16kb,
					_64kb
				};

				struct __attribute__((packed)) TableDescriptor {
					U64 validityDescriptor:1;
					U64 tableDescriptor:1;
					U64 mairIndex:3;
					U64 security:1;
					U64 accessPermission:2;
					U64 sharable:2;
					U64 accessFlag:1;
					U64 _reserved1:1;
					U64 address:42;
					U64 _reserved2:1;
					U64 privilegedExecuteNever:1;
					U64 unprivilegedExecuteNever:1;
					U64 softwareUse:4;
					U64 _reserved3:1;
				};
				static_assert(sizeof(TableDescriptor)==8);

				constexpr size_t get_granularity_size(Granularity granularity){
					switch(granularity){
						case Granularity::_4kb: return 4*1024;
						case Granularity::_16kb: return 16*1024;
						case Granularity::_64kb: return 64*1024;
					}
				}

				const auto granularity = Granularity::_16kb;
				static_assert(get_granularity_size(granularity)>=memory::pageSize);

				constexpr U8 physicalAddressSize = 36; //up to 48bit on armv8-a (up to 52bit on armv8.2-a). We'll set it to 36, which allows up to 64GB of ram to be addressed

				__attribute__((unused))
				constexpr U64 get_level_bitmask(U8 level){
					constexpr U64 mask = bitmask(0, physicalAddressSize-1);

					switch(granularity){
						case Granularity::_4kb:
							switch(level){
								case 0: return mask & 0b00000000'00000001'11111111'00000000'00000000'00000000'00000000'00000000; // 512GB
								case 1: return mask & 0b00000000'00000000'00000000'11111111'10000000'00000000'00000000'00000000; // 1GB
								case 2: return mask & 0b00000000'00000000'00000000'00000000'01111111'11000000'00000000'00000000; // 2MB
								case 3: return mask & 0b00000000'00000000'00000000'00000000'00000000'00111111'11100000'00000000; // 4KB
								default: return 0;
							}
						break;
						case Granularity::_16kb:
							switch(level){
								case 0: return mask & 0b00000000'00000001'00000000'00000000'00000000'00000000'00000000'00000000; // 128TB
								case 1: return mask & 0b00000000'00000000'11111111'11100000'00000000'00000000'00000000'00000000; // 64GB
								case 2: return mask & 0b00000000'00000000'00000000'00011111'11111100'00000000'00000000'00000000; // 32MB
								case 3: return mask & 0b00000000'00000000'00000000'00000000'00000011'11111111'10000000'00000000; // 16KB
								default: return 0;
							}
						break;
						case Granularity::_64kb:
							switch(level){
								case 0: return mask & 0b0;
								case 1: return mask & 0b00000000'00001111'11111111'00000000'00000000'00000000'00000000'00000000; // 4TB
								case 2: return mask & 0b00000000'00000000'00000000'11111111'11110000'00000000'00000000'00000000; // 512MB
								case 3: return mask & 0b00000000'00000000'00000000'00000000'00001111'11111111'00000000'00000000; // 64KB
								default: return 0;
							}
						break;
					}
				}

				constexpr unsigned level0Entries = 1<<bit_count(get_level_bitmask(0));
				constexpr unsigned level1Entries = 1<<bit_count(get_level_bitmask(1));
				constexpr unsigned level2Entries = 1<<bit_count(get_level_bitmask(2));
				constexpr unsigned level3Entries = 1<<bit_count(get_level_bitmask(3));

				constexpr U32 tableAlignment = (granularity==Granularity::_4kb?4:granularity==Granularity::_16kb?16:64)*1024;

				alignas(tableAlignment) TableDescriptor initialTableLower[level0Entries>1?level0Entries:level1Entries] = {0};
				alignas(tableAlignment) TableDescriptor initialTableUpper[level0Entries>1?level0Entries:level1Entries] = {0};
			}

			void init() {
				stdio::Section section("mmu::arch::arm64::init...");

				{
					stdio::Section section("table sizes");
					
					stdio::print_debug("0 = ", level0Entries);
					stdio::print_debug("1 = ", level1Entries);
					stdio::print_debug("2 = ", level2Entries);
					stdio::print_debug("3 = ", level3Entries);
				}

				enable();
			}
		
			void enable() {
				stdio::Section section("mmu::arch::arm64::enable...");

				const auto virtualAddressSpace = 64 - physicalAddressSize;

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
				switch(physicalAddressSize){
					case 32: //4GB
						ipsBits = 0b0000;
					break;
					case 36: //64GB
						ipsBits = 0b0001;
					break;
					case 40: //1TB
						ipsBits = 0b0010;
					break;
					case 42: //4TB
						ipsBits = 0b0011;
					break;
					case 44: //16TB
						ipsBits = 0b0100;
					break;
					case 48: //256TB
						ipsBits = 0b0101;
					break;
					case 52: //4PB
						ipsBits = 0b0110;
					break;
					default:
						stdio::print_error("Invalid physical address size: ", physicalAddressSize);
						ipsBits = 0b0001; //default to 64Bit
				}

				{
					stdio::Section section("tcr_el1");

					Tcr tcr_el1;
					tcr_el1.load_el1();

					tcr_el1.t0sz = virtualAddressSpace;
					tcr_el1.t1sz = virtualAddressSpace;

					tcr_el1.ips = ipsBits;

					tcr_el1.tbi0 = 0; //do not ignore first bit of ttbr0 address
					tcr_el1.tbi1 = 0; //do not ignore first bit of ttbr1 address

					tcr_el1.tg0 = tg0GranuleSize;
					tcr_el1.tg1 = tg1GranuleSize;

					tcr_el1.irgn0 = 0b01; //normal memory, inner write-back read-allocate write-allocate cacheable
					tcr_el1.irgn1 = 0b01; //normal memory, inner write-back read-allocate write-allocate cacheable

					tcr_el1.orgn0 = 0b01; //normal memory, outer write-back read-allocate write-allocate cacheable
					tcr_el1.orgn1 = 0b01; //normal memory, outer write-back read-allocate write-allocate cacheable

					tcr_el1.sh0 = 0b11; // inner sharable
					tcr_el1.sh1 = 0b11; // inner sharable

					tcr_el1.save_el1();
				}

				{
					stdio::Section section("sctlr_el1");

					Sctlr sctlr_el1;
					sctlr_el1.load_el1();

					sctlr_el1.mmuEnable = true;
					sctlr_el1.cacheEnable = true;
					sctlr_el1.exceptionBigEndian = true;

					sctlr_el1.save_el1();
				}

				{
					stdio::Section section("ttbr0_el1");

					Ttbr ttbr0_el1;
					ttbr0_el1.load_br0el1();

					ttbr0_el1.tableBaseAddress = (U64)&initialTableLower[0];
					ttbr0_el1.commonNotPrivate = false;

					ttbr0_el1.save_br0el1();
				}

				{
					stdio::Section section("ttbr1_el1");

					Ttbr ttbr1_el1;
					ttbr1_el1.load_br1el1();

					ttbr1_el1.tableBaseAddress = (U64)&initialTableUpper[0];
					ttbr1_el1.commonNotPrivate = false;

					ttbr1_el1.save_br1el1();
				}


				//TODO: MAIR_EL1
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
			}

			void set_userspace_mapping(MemoryMapping &memoryMapping) {
				Ttbr ttbr0_el1;
				ttbr0_el1.load_br0el1();

				ttbr0_el1.tableBaseAddress = (U64)&memoryMapping.initialTable;
				ttbr0_el1.commonNotPrivate = false;

				ttbr0_el1.save_br0el1();
			}

			/**/ MemoryMapping:: MemoryMapping():
				initialTable(memory::allocate_page()->physicalAddress),
				pageCount(0)
			{}

			/**/ MemoryMapping::~MemoryMapping(){
				clear();
				memory::kfree(initialTable);
			}

			namespace {
				inline void _clear_tables(TableDescriptor *table, unsigned size0, unsigned size1 = 0, unsigned size2 = 0, unsigned size3 = 0){
					for(unsigned i=0;i<size0;i++){
						auto &entry = table[i];
						auto *subTable = (TableDescriptor*)(U64)entry.address;
						if(!subTable) break;

						_clear_tables(subTable, size1, size2, size3);

						memory::free_page(*memory::get_memory_page(subTable));

						entry.address = 0;
					}
				}
			}

			void MemoryMapping::clear() {
				const auto initialTable = (TableDescriptor*)this->initialTable;

				unsigned size0;
				unsigned size1;
				unsigned size2;
				unsigned size3;

				if(level0Entries>1){
					size0 = level0Entries;
					size1 = level1Entries;
					size2 = level2Entries;
					size3 = level3Entries;
				}else{
					size0 = level1Entries;
					size1 = level2Entries;
					size2 = level3Entries;
					size3 = 0;
				}

				_clear_tables(initialTable, size0, size1, size2, size3);

				pageCount = 0;
			}

			bool MemoryMapping::add_pages(U32 count) {
				// round pagecount up to nearest whole granularity size
				const auto granSize = get_granularity_size(granularity);
				count = (count*memory::pageSize+granSize-1)/granSize*memory::pageSize;

				const auto pages = memory::allocate_pages(count);
				if(!pages) return false;

				//TODO

				return true;
			}
		}
	}
}
