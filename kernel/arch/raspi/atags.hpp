#pragma once

#include <common/types.hpp>

#if defined(ARCH_ARM32)
	#define HAS_ATAGS
#endif

namespace arch {
	namespace raspi {
		namespace atags {
			enum struct Tag: U32 {
				none       = 0x00000000,
				core       = 0x54410001,
				mem        = 0x54410002,
				videotext  = 0x54410003,
				ramdisk    = 0x54410004,
				initrd2    = 0x54410005,
				serial     = 0x54410006,
				revision   = 0x54410007,
				videolfb   = 0x54410008,
				cmdline    = 0x54410009
			};

			static const unsigned tag_max = (U32)Tag::mem;

			static char const * tag_name[] = {
				"none",
				"core",
				"mem",
			};

			struct Memory {
				U32 size;
				U32 start;
			};

			struct SerialNumber {
				U32 low;
				U32 high;
			};

			struct Atag {
				U32 tag_size;
				Tag tag;
				union {
					Memory memory;
					SerialNumber serialNumber;
				};
			};

			extern U32 mem_size;

			void init(const Atag *tags);
		}
	}
}

#include <common/stdlib.hpp>

template<> inline auto to_string(arch::raspi::atags::Tag tag) -> const char* {
	return (U32)tag<=arch::raspi::atags::tag_max?arch::raspi::atags::tag_name[(U32)tag]:to_string((U32)tag);
}
