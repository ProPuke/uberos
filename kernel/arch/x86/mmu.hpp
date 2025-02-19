#pragma once

#include <kernel/memory.hpp>
#include <kernel/mmu.hpp>

#include <common/types.hpp>

namespace mmu {
	struct Mapping;

	void activate(Mapping&);
	void deactivate();

	struct Mapping {
		/**/ Mapping();
		/**/~Mapping();

		auto map_physical_low(void*, MapOptions) -> void*;
		auto map_physical_low(void*, UPtr size, MapOptions) -> void*;
		auto map_physical_high(void*, MapOptions) -> void*;
		auto map_physical_high(void*, UPtr size, MapOptions) -> void*;

		void set_virtual_options(void *address, MapOptions);
		void set_virtual_options(void *address, UPtr size, MapOptions);

		void clear();

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
			U32 address:20; // bits 31-12 of address

			auto get_address() -> void* { return (void*)(address<<12); }
			void set_address(void *set) { debug::assert(!((UPtr)set&0b111111111111)); address = (UPtr)set>>12; }
		};
		static_assert(sizeof(TableEntry)==4);

		struct Table {
			TableEntry entry[1024];
		};
		static_assert(sizeof(Table)==4096);

		struct __attribute__((packed)) DirectoryEntry {
			/**/ DirectoryEntry(){}

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

					auto get_table() -> Table* { return (Table*)(address<<12); }
					void set_table(Table *set) { debug::assert(!((UPtr)set&0b111111111111)); address = (UPtr)set>>12; }
				} _4Kb;
				struct __attribute__((packed)) {
					bool isGlobal:1; // does not invalidate TLB upon MOV or CR3 (if PGE in CR4 enables global pages)
					U8 :3;
					bool pat:1;
					U64 address2:8; // bits 39-32 of address
					U8 :1;
					U64 address1:10; // bits 31-22 of address

					auto get_table() -> Table* { return (Table*)((U64)address2<<32|(U64)address1<<22); }
					void set_table(Table *set) { debug::assert(!((U64)set&0b1111111111111111111111)); address1 = (U64)set>>22; address2 = (U64)set>>32; }
				} _4Mb;
			};

			auto get_table() -> Table* { return is4Mb?_4Mb.get_table():_4Kb.get_table(); }
			void set_table(Table *set) { is4Mb?_4Mb.set_table(set):_4Kb.set_table(set); }
		};
		static_assert(sizeof(DirectoryEntry)==4);

		struct Directory {
			/**/ Directory(){}

			DirectoryEntry entry[1024];
		};
		static_assert(sizeof(Directory)==4096);

		Directory directory;

	protected:

		U16 nextLowDirectoryIndex = 0;
		U16 nextHighDirectoryIndex = 1023;
		U16 nextLowTableIndex = 0;
		U16 nextHighTableIndex = 1023;
	};
}
