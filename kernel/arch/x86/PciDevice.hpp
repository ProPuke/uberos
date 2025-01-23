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

	U32 baseAddress[6];

	auto readConfig8(uintptr_t offset) -> U8;
	auto readConfig16(uintptr_t offset) -> U16;
	auto readConfig32(uintptr_t offset) -> U32;
	void writeConfig8(uintptr_t offset, U8 value);
	void writeConfig16(uintptr_t offset, U16 value);
	void writeConfig32(uintptr_t offset, U32 value);
};
