#pragma once

#include <common/types.hpp>

namespace mmu {
	struct MemoryMapping;

	enum struct Caching {
		uncached, // no read or write caching (for general mmio)
		writeBack, // read cache, write cache (with delayed writes)
		writeThrough, // read cache, write cache (immediately written, but then cached for reads)
		writeCombining // no read caching, write cache (buffered and combined, but still written quickly, for high performance mmio)
	};

	struct MapOptions {
		bool isUserspace = true;
		bool isWritable = true;
		Caching caching = Caching::writeBack;
	};

	void init();

	namespace kernel {
		// in 32bit mode mapping high will start from the very top of the map and work backwards, but in 64bit or in other configs it may instead start halfway (with the first bit set) and then count up
		// this is implementation dependent

		auto map_physical_low(void*, MapOptions) -> void*;
		auto map_physical_low(void*, UPtr size, MapOptions) -> void*;
		auto map_physical_high(void*, MapOptions) -> void*;
		auto map_physical_high(void*, UPtr size, MapOptions) -> void*;

		void set_virtual_options(void*, MapOptions);
	}
}

#if defined(ARCH_ARM64)
	#include <kernel/arch/arm64/mmu.hpp>
#elif defined(ARCH_X86)
	#include <kernel/arch/x86/mmu.hpp>
#endif
