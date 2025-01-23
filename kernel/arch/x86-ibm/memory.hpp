#pragma once

namespace arch::x86_ibm {
	namespace memory {
		inline auto interuptVectorTable  = (void*)0x00000000;
		inline auto biosDataArea         = (void*)0x00000400;
		inline auto extendedBiosDataArea = (void*)0x00080000;
		inline auto videoDisplayMemory   = (void*)0x000a0000;
		inline auto videoBios            = (void*)0x000c0000;
		inline auto biosExpansions       = (void*)0x000c8000;
		inline auto motherboardBios      = (void*)0x000f0000;
	}
}
