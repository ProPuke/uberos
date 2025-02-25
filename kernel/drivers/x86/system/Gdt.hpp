#pragma once

#include <kernel/drivers/Hardware.hpp>

namespace driver::system {
	struct Gdt final: Hardware {
		DRIVER_INSTANCE(Gdt, 0xb8a16193, "gdt", "Global Descriptor Table", Hardware)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto can_stop_driver() -> bool override { return false; }
		auto can_restart_driver() -> bool override { return false; }

		U32 kernelCodeOffset;
		U32 kernelDataOffset;
		U32 userCodeOffset;
		U32 userDataOffset;

		enum struct DescriptorSize {
			_16bit,
			_32bit,
			_64bit
		};

		auto add_entry(U32 base, U32 limit, U8 access, DescriptorSize descriptorSize, bool granularity4k) -> U32;
		void set_entry(U32 i, U32 base, U32 limit, U8 access, DescriptorSize descriptorSize, bool granularity4k);
		void apply_entries();
	};
}
