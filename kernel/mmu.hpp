#pragma once

#include <common/types.hpp>

namespace mmu {
	struct MemoryMapping;

	enum struct RegionType {
		executable,
		memory,
		device,
		deviceMemory,
		test
	};

	extern MemoryMapping kernelMapping;

	void init();

	void enable();
	void disable();

	void set_kernelspace_mapping(MemoryMapping &memoryMapping);
	void set_userspace_mapping(MemoryMapping &memoryMapping);
}

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/mmu.hpp>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/mmu.hpp>
#else
	#error "Unsupported architecture"
#endif
