#pragma once

#include <common/types.hpp>

namespace arch {
	namespace x86 {
		namespace acpi {
			struct __attribute__((packed)) TableHeader {
				char signature[4];
				U32 length;
				U8 revision;
				U8 checksum;
				char oemId[6];
				char oemTableId[8];
				U32 oemRevision;
				char creatorId[4];
				U32 creatorRevision;
			};

			struct __attribute__((packed)) Sdt: TableHeader {
			};

			void init();

			auto get_entry_count() -> unsigned;
			auto get_entry(unsigned) -> Sdt*;
			auto find_entry_with_signature(char signature[4]) -> Sdt*;
		}
	}
}
