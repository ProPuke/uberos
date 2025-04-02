#pragma once

#include <common/types.hpp>

struct PciDevice {
	union {
		struct __attribute__((packed)) {
			U16 vendorId;
			U16 deviceId;
		};
		U32 id;
	};

	union {
		struct __attribute__((packed)) {
			U8 revision;
			U8 progIf;
			U8 subclassCode;
			U8 classCode;
		};
		U32 _class;
	};

	U8 bus;
	U8 device;
	U8 function;

	struct Bar {
		enum struct BarSize:U8 {
			_16bit,
			_32bit,
			_64bit
		};
		BarSize size;
		Physical64<void> memoryAddress;
		U64 memorySize;
		U32 ioAddress;
	};

	Bar bar[6];

	enum struct RegisterOffset:UPtr {
		vendor_id = 0x00,                               // 16
		device_id = 0x02,                               // 16
		command = 0x04,                                 // 16
		status = 0x06,                                  // 16
		revision_id = 0x08,                             // 8
		prog_if = 0x09,                                 // 8
		subclass = 0x0a,                                // 8
		_class = 0x0b,                                  // 8
		cache_line_size = 0x0c,                         // 8
		latency_timer = 0x0d,                           // 8
		header_type = 0x0e,                             // 8
		bist = 0x0f,                                    // 8
		bar0 = 0x10,                                    // 32
		bar1 = 0x14,                                    // 32
		bar2 = 0x18,                                    // 32
		bar3 = 0x1c,                                    // 32
		bar4 = 0x20,                                    // 32
		bar5 = 0x24,                                    // 32
		secondary_bus = 0x19,                           // 8
		subordinate_bus = 0x1a,                         // 8
		memory_base = 0x20,                             // 16
		memory_limit = 0x22,                            // 16
		prefetchable_memory_base = 0x24,                // 16
		prefetchable_memory_limit = 0x26,               // 16
		prefetchable_memory_base_upper_32_bits = 0x28,  // 32
		prefetchable_memory_limit_upper_32_bits = 0x2c, // 32
		subsystem_vendor_id = 0x2c,                     // 16
		subsystem_id = 0x2e,                            // 16
		expansion_rom_pointer = 0x30,                   // 32
		capabilities_pointer = 0x34,                    // 8
		interrupt_line = 0x3c,                          // 8
		interrupt_pin = 0x3d,                           // 8
	};

	auto readConfig8(UPtr offset) -> U8;
	auto readConfig16(UPtr offset) -> U16;
	auto readConfig32(UPtr offset) -> U32;
	void writeConfig8(UPtr offset, U8 value);
	void writeConfig16(UPtr offset, U16 value);
	void writeConfig32(UPtr offset, U32 value);

	void enable_io_space(bool);
	void enable_memory_space(bool);
	void enable_bus_mastering(bool);
	void enable_interrupts(bool);
};
