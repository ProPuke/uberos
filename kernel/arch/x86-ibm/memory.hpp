#pragma once

#include <kernel/IdentityMappedPointer.hpp>

namespace arch::x86_ibm {
	namespace memory {
		inline auto interuptVectorTable  = IdentityMapped<void>{0x00000000};
		inline auto biosDataArea         = IdentityMapped<void>{0x00000400};
		inline auto extendedBiosDataArea = IdentityMapped<void>{0x00080000};
		inline auto videoDisplayMemory   = IdentityMapped<void>{0x000a0000};
		inline auto videoBios            = IdentityMapped<void>{0x000c0000};
		inline auto biosExpansions       = IdentityMapped<void>{0x000c8000};
		inline auto motherboardBios      = IdentityMapped<void>{0x000f0000};
	}
}
