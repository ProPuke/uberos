#include "mmu.hpp"

#include <common/stdlib.hpp>

#include <kernel/arch/raspi/mailbox.hpp>
#include <kernel/memory.hpp>
#include <kernel/stdio.hpp>

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
				const inline U32 tlb_alignment = 4096; //4KB (16384KB on arm32)
				const U32 level1_blocksize = 1 << 21; //Each Block is 2Mb in size (1MB for arm32)
				const U32 pageTableEntryCount = 8192;

				enum struct MemoryAttribute {
					device_ngnrne,
					device_ngnre,
					device_gre,
					normal_nc,
					normal
				};

				//arm32
				// enum struct MemoryAttribute {
				// 	device_ns = 0x10412, // device no share (strongly ordered)
				// 	device    = 0x10416, // device + shareable
				// 	normal    = 0x1040E, // normal cache + shareable
				// 	normal_ns = 0x1040A, // normal cache no share
				// 	normal_xn = 0x1041E  // normal cache + shareable and execute never
				// };

				enum struct EntryType {
					none,
					blockTable,
					pageTable
				};

				union VmsaV8_64Descriptor {
					enum struct Stage2S2ap {
						none,
						noread_el0, // no read access for EL0
						no_write,   // no write access
					};
					enum struct Stage2Sh {
						none,
						unpredictable,
						outer,
						inner,
					};
					enum struct ApTable {
						allAccess,
						no_el0,                            // Access at EL0 not permitted, regardless of permissions in subsequent levels of lookup
						no_write,                          // Write access not permitted, at any Exception level, regardless of permissions in subsequent levels of lookup
						no_write_el0_read                  // Write access not permitted,at any Exception level, Read access not permitted at EL0.
					};
					struct {
						EntryType entryType : 2;                     // @0-1 1 for a block table, 3 for a page table
							
							// These are only valid on BLOCK DESCRIPTOR
							MemoryAttribute memAttr : 4;       // @2-5
							Stage2S2ap s2ap : 2;               // @6-7
							Stage2Sh sh : 2;                   // @8-9
							U64 af : 1;                        // @10 Accessable flag

						U64 _reserved11 : 1;                   // @11 Set to 0
						U64 address : 36;                      // @12-47 36 Bits of address
						U64 _reserved48_51 : 4;                // @48-51 Set to 0
						U64 contiguous : 1;                    // @52 Contiguous
						U64 pxn : 1;                           // @53 Set to 0
						U64 uxn : 1;                           // @54 No execute if bit set
						U64 _reserved55_58 : 4;                // @55-58 Set to 0
						
						U64 pxnTable : 1;                      // @59 Never allow execution from a lower EL level 
						U64 uxnTable : 1;                      // @60 Never allow translation from a lower EL level
						ApTable apTable : 2;                   // @61-62 AP Table control .. see enumerate options
						U64 nsTable : 1;                       // @63 Secure state, for accesses from Non-secure state this bit is RES0 and is ignored
					};

					U64 raw;                                  // @0-63 Raw access to all 64 bits via this union
				};

				/* First Level Page Table for 1:1 mapping */
				// static Reg pageTable1to1[pageTableEntryCount] = { 0 };
				// static Reg __attribute__((aligned(tlb_alignment))) pageTable1to1[pageTableEntryCount] = { 0 };
				/* First Level Page Table for virtual mapping */
				static Reg __attribute__((aligned(tlb_alignment))) page_table_virtualmap[pageTableEntryCount] = { 0 };
				/* First Level Page Table for virtual mapping */
				static Reg __attribute__((aligned(tlb_alignment))) stage2virtual[512] = { 0 };

				static VmsaV8_64Descriptor *stage2map1to1 = nullptr;
				static VmsaV8_64Descriptor *stage3virtual = nullptr;

				void enable_mmu_tables(VmsaV8_64Descriptor *translationTable0, VmsaV8_64Descriptor *translationTable1 = nullptr) {
					asm volatile("dsb sy");

					asm volatile("msr mair_el1, %0" : : "r" (0
						| (0x00ul << ((U64)MemoryAttribute::device_ngnrne * 8))
						| (0x04ul << ((U64)MemoryAttribute::device_ngnre * 8))
						| (0x0cul << ((U64)MemoryAttribute::device_gre * 8))
						| (0x44ul << ((U64)MemoryAttribute::normal_nc * 8))
						| (0xfful << ((U64)MemoryAttribute::normal * 8))
					));
					asm volatile("msr ttbr0_el1, %0" : : "r" (translationTable0));
					if(translationTable1){
						asm volatile("msr ttbr1_el1, %0" : : "r" (translationTable1));
					}
					asm volatile("isb");

					{
						const U64 tcrBits = 25; // 512GB per region 2^(64-tcrBits)

						U64 tcr_el1;
						asm volatile("mrs %0, tcr_el1" : "=r" (tcr_el1));
						tcr_el1 |= (0b00ll  << 37); // TBI=0, no tagging
						tcr_el1 |= (0b000ll << 32); // IPS= 32 bit ... 000 = 32bit, 001 = 36bit, 010 = 40bit
						tcr_el1 |= (0b10ll  << 30); // TG1=4k ... options are 10=4KB, 01=16KB, 11=64KB ... t care differs from TG0
						tcr_el1 |= (0b11ll  << 28); // SH1=3 inner ... options 00 = Non-shareable, 01 = INVALID, 10 = Outer Shareable1 = Inner Shareable
						tcr_el1 |= (0b01ll  << 26); // ORGN1=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write thru cacheab 11 = Write Back Non-cacheable
						tcr_el1 |= (0b01ll  << 24); // IRGN1=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write thru cacheable, 11 = Write Back Non-cacheable
						tcr_el1 |= (0b0ll   << 23); // EPD1 ... Translation table walk disable for translations using TTBR1_EL1  0 = walk, 1 = generate fault
						tcr_el1 |= (tcrBits << 16); // T1SZ
						tcr_el1 |= (0b00ll  << 14); // TG0=4k  ... options are 00=4KB, 01=64KB, 10=16KB,  ... take c differs from TG1
						tcr_el1 |= (0b11ll  << 12); // SH0=3 inner ... .. options 00 = Non-shareable, 01 = INVALID, 10 = Outer Shareable1 = Inner Shareable
						tcr_el1 |= (0b01ll  << 10); // ORGN0=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write thru cacheable, = Write Back Non-cacheable
						tcr_el1 |= (0b01ll  <<  8); // IRGN0=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write thru cacheable, 11 = Write Back Non-cacheable
						tcr_el1 |= (0b0ll   <<  7); // EPD0  ... Translation table walk disable for translations using TTBR0_EL1  0 = walk, 1 = generate fault
						tcr_el1 |= (tcrBits <<  0); // T0SZ
						asm volatile("msr tcr_el1, %0" : : "r" (tcr_el1));
					}

					asm volatile("isb");

					{
						U64 sctlr_el1;
						asm volatile("mrs %0, sctlr_el1" : "=r" (sctlr_el1));
						sctlr_el1 |= (1<<22); // EIS, Exception Entry is Context Synchronizing
						sctlr_el1 |= (1<<11); // EOS, Exception Exit is Context Synchronizing
						sctlr_el1 |= (1<<12); // I, instruction cache enable. This is an enable bit for instruction caches at EL0 and EL1
						// sctlr_el1 |= (1<<4);  // SA0, stack alignment check enable for EL0
						// sctlr_el1 |= (1<<3);  // SA, stack alignment check enable
						sctlr_el1 |= (1<<2);  // C, data cache enable. This is an enable bit for data caches at EL0 and EL1
						// sctlr_el1 |= (1<<1);  // A, alignment check enable bit
						sctlr_el1 |= (1<<0);  // M, enable MMU
						asm volatile("msr sctlr_el1, %0" : : "r" (sctlr_el1) : "memory");
					}
				}

				void invalidate_tables() {
					asm volatile("dsb ishst"); //ensure all writes completed
					asm volatile("tlbi alle0" : : : "memory"); // clear all table cache for exception level 0
					asm volatile("dsb ish"); // ensure invalidation completed
					asm volatile("isb"); // sync context
				}
			}

			void init() {
				stdio::Section section("mmu::arch::arm64::init");

				stage2map1to1 = (VmsaV8_64Descriptor*)memory::allocate_page()->physicalAddress;
				bzero(stage2map1to1, sizeof(stage2map1to1));

				stage3virtual = (VmsaV8_64Descriptor*)memory::allocate_page()->physicalAddress;
				bzero(stage3virtual, sizeof(stage3virtual));

				mailbox::PropertyMessage tags[4];
				tags[0].tag = mailbox::PropertyTag::get_arm_memory;
				tags[1].tag = mailbox::PropertyTag::get_vc_memory;
				tags[2].tag = mailbox::PropertyTag::null_tag;

				if(!send_messages(tags)){
					stdio::print_error("Error: Unable to query for arm/vc memory");
					return;
				}

				auto vs_address = tags[1].data.memory.address / level1_blocksize;

				(((addr) >> 16) & 0xFFFFFFFF)

				U32 base = 0;

				// up to vc4 ram start
				for(;base<vs_address;base++){
					stage2map1to1[base].address = base << (21-12);
					stage2map1to1[base].af = 1;
					stage2map1to1[base].sh = VmsaV8_64Descriptor::Stage2Sh::inner_shareable;
					stage2map1to1[base].memAttr = MemoryAttribute::normal;
					stage2map1to1[base].entryType = EntryType::blockTable;
				}

				for(;base<(U32)mmio::Address::peripheral_base/level1_blocksize;base++){
					stage2map1to1[base].address = base << (21-12);
					stage2map1to1[base].af = 1;
					stage2map1to1[base].memAttr = MemoryAttribute::normal_nc;
					stage2map1to1[base].entryType = EntryType::blockTable;
				}

				// peripherals
				for(;base<((U32)mmio::Address::peripheral_base+(U32)mmio::Address::peripheral_length)/level1_blocksize;base++){
					stage2map1to1[base].address = base << (21-12);
					stage2map1to1[base].af = 1;
					stage2map1to1[base].memAttr = MemoryAttribute::device_ngnrne;
					stage2map1to1[base].entryType = EntryType::blockTable;
				}

				// // 2MB for mailboxes (0x40000000)
				// // shared device, never execute
				// stage2map1to1[base].address = 512 << (21-12);
				// stage2map1to1[base].af = 1;
				// stage2map1to1[base].memAttr = MemoryAttribute::device_ngnrne;
				// stage2map1to1[base].entryType = EntryType::blockTable;

				// level 1 has just 2 valid entries mapping the each 1GB in stage2 to cover the 2GB
				// pageTable1to1[0] = (0x8000000000000000) | (uintptr_t)&stage2map1to1[0] | 3;
				// pageTable1to1[1] = (0x8000000000000000) | (uintptr_t)&stage2map1to1[512] | 3;


				// initialize virtual mapping for TTBR1 .. basic 1 page  .. 512 entries x 4K pages
				// 2MB of ram memory memory  0xFFFFFFFFFFE00000 to 0xFFFFFFFFFFFFFFFF

				// stage2 virtual has just 1 valid entry (the last) of the 512 entries pointing to the Stage3 virtual table
				// stage 3 starts as all invalid they will be added by mapping call
				//stage2virtual[511] = (VMSAv8_64_DESCRIPTOR){ .NSTable = 1,.Address = &Stage3virtual[0] >> 12,.EntryType = EntryType.pageTable };
				stage2virtual[511] = (0x8000000000000000) | (uintptr_t)&stage3virtual[0] | 3;

				// stage1 virtual has just 1 valid entry (the last) of 512 entries pointing to the Stage2 virtual table
				page_table_virtualmap[511] = (0x8000000000000000) | (uintptr_t)&stage2virtual[0] | 3;

				stdio::print_info(base, " table entries");

				stdio::print_info("Enabling...");
				enable();
			}
		
			void enable() {
				// enable_mmu_tables(&stage2map1to1[0], &page_table_virtualmap[0]);
				enable_mmu_tables(&stage2map1to1[0], &stage2virtual);
			}

			void disable() {
				{
					U64 sctlr_el1;
					asm volatile("mrs %0, sctlr_el1" : "=r" (sctlr_el1));
					sctlr_el1 &= ~(0
						| (1<<2) // C, data cache enable. Data caches at EL0 and EL1
						| (1<<0) // M, MMU
					);
					asm volatile("msr sctlr_el1, %0" : : "r" (sctlr_el1) : "memory");
				}
				
				asm volatile("dsb sy");
				asm volatile("isb");

				//TODO: clean data cache
				//TODO: invalidate data cache

				asm volatile ("tlbi vmalle1" : : : "memory");

				asm volatile("dsb sy");
				asm volatile("isb");
			}
		}
	}
}

// #if __aarch64__ == 1
// RegType_t virtualmap (uint32_t phys_addr, uint8_t memattrs) {
// 	uint64_t addr = 0;
// 	for (int i = 0; i < 512; i++)
// 	{
// 		if (Stage3virtual[i].Raw64 == 0) {							// Find the first vacant stage3 table slot
// 			uint64_t offset;
// 			Stage3virtual[i] = (VMSAv8_64_DESCRIPTOR) { .Address = (uintptr_t)phys_addr << (21 - 12), .AF = 1, .MemAttr = memattrs, .EntryType = 3 };
// 			__asm volatile ("dmb sy" ::: "memory");
// 			offset = ((512 - i) * 4096) - 1;
// 			addr = 0xFFFFFFFFFFFFFFFFul;
// 			addr = addr - offset;
// 			return(addr);
// 		}
// 	}
// 	return (addr);													// error
// }
// #endif
