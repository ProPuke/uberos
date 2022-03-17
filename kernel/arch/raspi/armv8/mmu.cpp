#include <kernel/mmu.hpp>
#include "../hwquery.hpp"
#include "../mmio.hpp"

namespace hwquery {
	using namespace arch::raspi;
}

namespace mmio {
	using namespace arch::raspi;
}

extern U8 __text_start;
extern U8 __text_end;

namespace mmu {
	extern MemoryMapping kernelMapping;

	namespace {
		void* round_address_down(void *address) {
			return (void*)((U64)address/memory::pageSize*memory::pageSize);
		}
		void* round_address_up(void *address) {
			return (void*)(((U64)address+memory::pageSize-1)/memory::pageSize*memory::pageSize);
		}
	}

	void init_kernelMap() {
		void *text_start = round_address_down(&__text_start);
		void *text_end = round_address_up(&__text_end);
		void *video_start = round_address_down(hwquery::videoMemoryStart);
		void *video_end = round_address_up((void*)((U64)hwquery::videoMemoryStart+hwquery::videoMemory));
		void *peripheral_start = round_address_down((void*)mmio::Address::gpu_peripheral_base);
		void *peripheral_end = round_address_up((void*)((U64)mmio::Address::gpu_peripheral_base+(U64)mmio::Address::gpu_peripheral_length));

		// kernelMapping.add_mapping(nullptr, 0x7'ffff'ffff/memory::pageSize, RegionType::test);

		kernelMapping.add_mapping(nullptr, text_start, RegionType::memory);
		kernelMapping.add_mapping(text_start, text_end, RegionType::executable);
		kernelMapping.add_mapping(text_end, video_start, RegionType::memory);
		kernelMapping.add_mapping(video_start, video_end, RegionType::deviceMemory);
		kernelMapping.add_mapping(video_end, peripheral_start, RegionType::device);
		kernelMapping.add_mapping(peripheral_start, peripheral_end, RegionType::device);
		kernelMapping.add_mapping(peripheral_end, (void*)0xf'ffff'ffff, RegionType::memory);

		// 0xff800000 peripheral_end
		// 0xfe000000 peripheral_start
	}
}
