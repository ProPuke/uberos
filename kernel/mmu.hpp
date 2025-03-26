#pragma once

#include <kernel/PhysicalPointer.hpp>

#include <common/Try.hpp>
#include <common/types.hpp>

namespace mmu {
	struct Mapping;

	enum struct Caching {
		uncached, // no read or write caching (for general mmio)
		writeBack, // read cache, write cache (with delayed writes)
		writeThrough, // read cache, write cache (immediately written, but then cached for reads)
		writeCombining // no read caching, write cache (buffered and combined, but still written quickly, for high performance mmio)
	};

	struct MapOptions {
		bool isUserspace = false;
		bool isWritable = true;
		bool isExecutable = false; // TODO: implement on x64 and other archs
		Caching caching = Caching::writeBack;
	};

	//TODO: function to indicate if isExecutable is supported, or if it will simply be true everywhere

	void init();

	namespace kernel {
		void _lock();
		void _unlock();

		// note that low mappings should ALWAYS return sequential addresses when called repeatedly within the same transaction
		// high mappings should also ALWAYS return reverse sequential address when called repeatedly within the same transaction
		// (that way you map differing physical pages as sequential by calling multiple within a transaction)

		auto _map_physical_low(Physical<void>, MapOptions) -> void*;
		auto _map_physical_low(Physical<void>, UPtr size, MapOptions) -> void*;
		auto _map_physical_high(Physical<void>, MapOptions) -> void*;
		auto _map_physical_high(Physical<void>, UPtr size, MapOptions) -> void*;

		// auto unmap_virtual_high(void *virtualAddress, UPtr size) -> void*;

		void _set_virtual_target(void*, Physical<void>, UPtr size);
		void _set_virtual_options(void*, MapOptions);
		void _set_virtual_options(void*, UPtr size, MapOptions);
		auto _get_virtual_options(void*) -> Try<MapOptions>;

		auto _get_physical(void*) -> Physical<void>;

		struct Transaction: NonCopyable<Transaction> {
			/**/ Transaction() { _lock(); }
			/**/~Transaction() { _unlock(); }
	
			auto map_physical_low(Physical<void> physical, MapOptions options) -> void* { return _map_physical_low(physical, options); }
			auto map_physical_low(Physical<void> physical, UPtr size, MapOptions options) -> void* { return _map_physical_low(physical, size, options); }
			auto map_physical_high(Physical<void> physical, MapOptions options) -> void* { return _map_physical_high(physical, options); }
			auto map_physical_high(Physical<void> physical, UPtr size, MapOptions options) -> void* { return _map_physical_high(physical, size, options); }
	
			void set_virtual_target(void *address, Physical<void> physical, UPtr size) { return _set_virtual_target(address, physical, size); }
			void set_virtual_options(void *address, MapOptions options) { return _set_virtual_options(address, options); }
			void set_virtual_options(void *address, UPtr size, MapOptions options) { return _set_virtual_options(address, size, options); }
			auto get_virtual_options(void *address) -> Try<MapOptions> { return _get_virtual_options(address); }
	
			auto get_physical(void *physical) -> Physical<void> { return _get_physical(physical); }
		};

		inline auto transaction() -> Transaction { return {}; }
	}

}

#if defined(ARCH_ARM64)
	#include <kernel/arch/arm64/mmu.hpp>
#elif defined(ARCH_X86)
	#include <kernel/arch/x86/mmu.hpp>
#endif
