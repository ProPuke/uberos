#pragma once

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/mmu/MemoryMapping.hpp>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/mmu/MemoryMapping.hpp>
#endif


namespace mmu {
	#if defined(ARCH_ARM32)
		using namespace ::mmu::arch::arm32;
	#elif defined(ARCH_ARM64)
		using namespace ::mmu::arch::arm64;
	#endif
}
