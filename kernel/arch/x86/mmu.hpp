#pragma once

#include <kernel/Lock.hpp>
#include <kernel/memory.hpp>
#include <kernel/mmu.hpp>
#include <kernel/PhysicalPointer.hpp>

#include <common/Try.hpp>
#include <common/types.hpp>

namespace mmu {
	struct Mapping;

	void init();

	void activate(Mapping&);
	void deactivate();

	struct Mapping {
		/**/ Mapping();
		/**/~Mapping();

		void _lock();
		void _unlock();

		auto _map_physical_low(Physical<void>, MapOptions) -> void*;
		auto _map_physical_low(Physical<void>, UPtr size, MapOptions) -> void*;
		auto _map_physical_high(Physical<void>, MapOptions) -> void*;
		auto _map_physical_high(Physical<void>, UPtr size, MapOptions) -> void*;

		// auto unmap_virtual_high(void *virtualAddress, UPtr size) -> void*;

		void _set_virtual_target(void *address, Physical<void>, UPtr size);
		void _set_virtual_options(void *address, MapOptions);
		void _set_virtual_options(void *address, UPtr size, MapOptions);
		auto _get_virtual_options(void *address) -> Try<MapOptions>;

		auto _get_physical(void*) -> Physical<void>;

		// void clear();

		struct Transaction: NonCopyable<Transaction> {
			Mapping &mapping;

			/**/ Transaction(Mapping &mapping):mapping(mapping) { mapping._lock(); }
			/**/~Transaction() { mapping._unlock(); }

			auto map_physical_low(Physical<void> physical, MapOptions options) -> void* { return mapping._map_physical_low(physical, options); }
			auto map_physical_low(Physical<void> physical, UPtr size, MapOptions options) -> void* { return mapping._map_physical_low(physical, size, options); }
			auto map_physical_high(Physical<void> physical, MapOptions options) -> void* { return mapping._map_physical_high(physical, options); }
			auto map_physical_high(Physical<void> physical, UPtr size, MapOptions options) -> void* { return mapping._map_physical_high(physical, size, options); }
	
			void set_virtual_target(void *address, Physical<void> physical, UPtr size) {return mapping._set_virtual_target(address, physical, size); }
			void set_virtual_options(void *address, MapOptions options) {return mapping._set_virtual_options(address, options); }
			void set_virtual_options(void *address, UPtr size, MapOptions options) {return mapping._set_virtual_options(address, size, options); }
	
			auto get_physical(void *physical) -> Physical<void> { return mapping._get_physical(physical); }
		};

		auto transaction() -> Transaction { return {*this}; }

		struct FreedEntry: LListItem<FreedEntry> {
			void *virtualAddress;
			UPtr size;
		};

		struct __attribute__((packed)) TableEntry {
			bool isPresent:1;
			bool isWritable:1;
			bool isUserspaceAccessible:1;
			bool pat_writeThroughCaching:1; // vs write-back caching ("PWT")
			bool pat_disableCache:1; // ("PCD")
			bool isAccessed:1;
			bool isDirty:1;
			bool pat:1;
			bool isGlobal:1; // does not invalidate TLB upon MOV or CR3 (if PGE in CR4 enables global pages)

			U32 _:3;

			U32 address:20; // bits 31-12 of address

			auto get_address() -> Physical32<void> { return Physical32<void>{(U32)(address<<12)}; }
			void set_address(Physical32<void> set) { debug::assert(!((UPtr)set.address&0b111111111111)); address = set.address>>12; }
		};
		static_assert(sizeof(TableEntry)==4);

		struct Table {
			TableEntry entry[1024];
		};
		static_assert(sizeof(Table)==4096);

		struct __attribute__((packed)) DirectoryEntry {
			bool isPresent:1;
			bool isWritable:1;
			bool isUserspaceAccessible:1;
			bool pat_writeThroughCaching:1;
			bool pat_disableCache:1;
			bool isAccessed:1;
			bool isDirty:1; // 4Mb only
			bool is4Mb:1; // true

			union __attribute__((packed)) {
				struct __attribute__((packed)) {
					U8 :4;
					U32 address:20; // bits 31-12 of address

					auto get_table() -> Physical32<Table> { return Physical32<Table>{address?(U32)(address<<12):0}; }
					void set_table(Physical32<Table> set) { debug::assert(!(set.address&0b111111111111)); address = set.address>>12; }
				} _4Kb;
				struct __attribute__((packed)) {
					bool isGlobal:1; // does not invalidate TLB upon MOV or CR3 (if PGE in CR4 enables global pages)
					U8 :3;
					bool pat:1;
					U64 address2:8; // bits 39-32 of address
					U8 :1;
					U64 address1:10; // bits 31-22 of address

					auto get_table() -> Physical64<Table> { return Physical64<Table>{address1||address2?(U64)address2<<32|(U64)address1<<22:0}; }
					void set_table(Physical64<Table> set) { debug::assert(!(set.address&0b1111111111111111111111)); address1 = set.address>>22; address2 = set.address>>32; }
				} _4Mb;
			};

			auto get_table() -> Physical<Table> { return is4Mb?_4Mb.get_table().as_native():_4Kb.get_table().as_native(); }
			void set_table(Physical<Table> set) { is4Mb?_4Mb.set_table(set.as_size<U64>()):_4Kb.set_table(set.as_size<U32>()); }
		};
		static_assert(sizeof(DirectoryEntry)==4);

		struct Directory {
			/**/ Directory(){}

			DirectoryEntry entry[1024];
		};
		static_assert(sizeof(Directory)==4096);

		Directory directory;

	protected:

		friend void mmu::init();

		Table *nextFreeHighTable = nullptr; // a preallocated table, so that we can allocate another table when we run out (as we can't allocate when out of tables)

		U16 nextLowDirectoryIndex = 0;
		U16 nextHighDirectoryIndex = 1023;
		U16 nextLowTableIndex = 0;
		U16 nextHighTableIndex = 1023-1; // NOTE: we skip the final table. This avoids problems with x < table+tableSize, where the rightside would wrap around to 0x0 and thus always be false

		Lock<LockType::flat> lock;
	};
}
