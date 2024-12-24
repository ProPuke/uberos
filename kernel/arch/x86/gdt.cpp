#include "gdt.hpp"

#include <common/types.hpp>

namespace arch {
	namespace x86 {
		namespace gdt {
			struct __attribute__((packed)) Gdt {
				U16 limit; // the last valid byte (size-1)
				// U32 entriesAddress;
				void *entries;
			};

			struct __attribute__((packed)) Entry {
				U16 limit_low;
				U16 base_low;
				U8 base_middle;
				U8 access;
				U8 granularity;
				U8 base_high;
			};


			Entry entries[5];

			Gdt gdt {
				sizeof(entries)-1,
				// (size_t)&entries
				&entries
			};

			void set_entry(U8 i, U32 base, U32 limit, U8 access, U8 granularity) {
				auto &entry = entries[i];

				entry.base_low = base & 0xFFFF;
				entry.base_middle = base>>16 & 0xFF;
				entry.base_high = base>>24 & 0xFF;

				entry.limit_low = limit & 0xFFFF;
				entry.granularity = limit>>16 & 0x0F;

				entry.granularity |= granularity & 0xF0;
				entry.access = access;
			}

			void apply_entries() {
				asm(
					"lgdt %0"
					:
					: "m"(gdt)
					: "memory"
				);
			}

			U32 kernelCodeOffset = 1 * sizeof(Entry);
			U32 kernelDataOffset = 2 * sizeof(Entry);
			U32 userCodeOffset = 3 * sizeof(Entry);
			U32 userDataOffset = 4 * sizeof(Entry);

			void init() {
				set_entry(0, 0x000000000, 0x00000000, 0x00, 0x00); // null descriptor
				set_entry(kernelCodeOffset/sizeof(Entry), 0x000000000, 0xffffffff, 0x9a, 0xcf); // kernel mode code segment
				set_entry(kernelDataOffset/sizeof(Entry), 0x000000000, 0xffffffff, 0x92, 0xcf); // kernel mode data segment
				set_entry(userCodeOffset/sizeof(Entry), 0x000000000, 0xffffffff, 0xfa, 0xcf); // user mode code segment
				set_entry(userDataOffset/sizeof(Entry), 0x000000000, 0xffffffff, 0xf2, 0xcf); // user mode data segment
				apply_entries();
			}
		}
	}
}
