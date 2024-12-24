#pragma once

#include <common/types.hpp>

namespace arch {
	namespace x86 {
		namespace gdt {
			extern U32 kernelCodeOffset;
			extern U32 kernelDataOffset;
			extern U32 userCodeOffset;
			extern U32 userDataOffset;

			void set_entry(U8 i, U32 base, U32 limit, U8 access, U8 granularity);
			void apply_entries();

			void init();
		}
	}
}
