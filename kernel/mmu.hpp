#pragma once

#include <common/types.hpp>

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/mmu.hpp>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/mmu.hpp>
#else
	#error "Unsupported architecture"
#endif

namespace mmu {
	// struct MemoryMapping;
	
	#if defined(ARCH_ARM32)
		using namespace ::mmu::arch::arm32;
	#elif defined(ARCH_ARM64)
		using namespace ::mmu::arch::arm64;
	#endif
}
