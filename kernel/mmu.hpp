#pragma once

#include <common/types.hpp>

#if defined(ARCH_ARM64)
	#define KERNEL_MMU
#endif

#ifdef KERNEL_MMU
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
		auto is_enabled() -> bool;

		void set_kernelspace_mapping(MemoryMapping &memoryMapping);
		void set_userspace_mapping(MemoryMapping &memoryMapping);
	}

	#include "mmu.inl"

	#if defined(ARCH_ARM64)
		#include <kernel/arch/arm64/mmu.hpp>
	#endif
#endif
