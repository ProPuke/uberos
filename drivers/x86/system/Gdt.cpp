#include "Gdt.hpp"

#include <kernel/Lock.hpp>

namespace driver::system {
	namespace {
		Lock<LockType::flat> lock("gdt");

		struct __attribute__((packed)) Gdtr {
			U16 limit; // the last valid byte (size-1)
			// U32 entriesAddress;
			void *entries;
		};

		struct __attribute__((packed)) Entry {
			U16 limit_low;
			U16 base_low;
			U8 base_middle;
			U8 access;

			U8 limit_high:4;

			bool flag_reserved:1;
			bool flag_64bit:1; // descriptor is 64bit segment (incompat with `size`)
			bool flag_32bit:1; // descriptor is 32bit protected mode segment (vs 16bit protected)
			bool flag_granularity4k:1; // 4KB (vs 1 byte) scaling of limit
			U8 base_high;

			// U8 granularity;
			// U8 base_high;
		};

		U16 count = 0;
		const U16 max = 32;

		Entry entries[max];

		Gdtr gdtr {
			U16(count*sizeof(Entry)-1),
			// (size_t)&entries
			&entries
		};

		void _set_entry(U32 i, U32 base, U32 limit, U8 access, Gdt::DescriptorSize descriptorSize, bool granularity4k) {
			auto &entry = entries[i/sizeof(Entry)];

			entry.base_low = base & 0xFFFF;
			entry.base_middle = base>>16 & 0xFF;
			entry.base_high = base>>24 & 0xFF;

			entry.access = access;

			entry.limit_low = limit & 0xFFFF;
			entry.limit_high = limit>>16 & 0x0F;
			// entry.granularity = limit>>16 & 0x0F;

			entry.flag_reserved = false;
			switch(descriptorSize){
				case Gdt::DescriptorSize::_16bit:
					entry.flag_64bit = false;
					entry.flag_32bit = false;
				break;
				case Gdt::DescriptorSize::_32bit:
					entry.flag_64bit = false;
					entry.flag_32bit = true;
				break;
				case Gdt::DescriptorSize::_64bit:
					entry.flag_64bit = true;
					entry.flag_32bit = false;
				break;
			}
			entry.flag_granularity4k = granularity4k;
		}

		auto _add_entry(U32 base, U32 limit, U8 access, Gdt::DescriptorSize descriptorSize, bool granularity4k) -> U32 {
			auto id = count++ * sizeof(Entry);
			_set_entry(id, base, limit, access, descriptorSize, granularity4k);
			gdtr.limit = count*sizeof(Entry)-1;

			return id;
		}

		void _apply_entries() {
			asm volatile(
				"lgdt %0\n"
				"jmp 0x08:after%=\n"
				"after%=:\n"
				:
				: "m"(gdtr)
				: "memory"
			);
		}
	}

	auto Gdt::_on_start() -> Try<> {
		Lock_Guard guard(lock);

		kernelCodeOffset = 0;
		kernelDataOffset = 0;
		userCodeOffset = 0;
		userDataOffset = 0;

		_add_entry(0x000000000, 0x00000000, 0x00, DescriptorSize::_32bit, true); // null descriptor
		kernelCodeOffset = _add_entry(0x000000000, 0xffffffff, 0x9a, DescriptorSize::_32bit, true); // kernel mode code segment
		kernelDataOffset = _add_entry(0x000000000, 0xffffffff, 0x92, DescriptorSize::_32bit, true); // kernel mode data segment
		userCodeOffset = _add_entry(0x000000000, 0xffffffff, 0xfa, DescriptorSize::_32bit, true); // user mode code segment
		userDataOffset = _add_entry(0x000000000, 0xffffffff, 0xf2, DescriptorSize::_32bit, true); // user mode data segment
		_apply_entries();

		return {};
	}

	void Gdt::set_entry(U32 i, U32 base, U32 limit, U8 access, DescriptorSize descriptorSize, bool granularity4k) {
		Lock_Guard guard(lock);

		return _set_entry(i, base, limit, access, descriptorSize, granularity4k);
	}

	auto Gdt::add_entry(U32 base, U32 limit, U8 access, DescriptorSize descriptorSize, bool granularity4k) -> U32 {
		Lock_Guard guard(lock);

		return _add_entry(base, limit, access, descriptorSize, granularity4k);
	}

	void Gdt::apply_entries() {
		Lock_Guard guard(lock);

		return _apply_entries();
	}
}
