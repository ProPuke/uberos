#include "Hpet.hpp"

namespace driver {
	namespace timer {
		namespace {
			struct __attribute__((packed)) AddressStructure {
				U8 address_space_id;    // 0 - system memory, 1 - system I/O
				U8 register_bit_width;
				U8 register_bit_offset;
				U8 reserved;
				U64 address;
			};

			struct __attribute__((packed)) DescriptionTableHeader {
				char signature[4];    // 'HPET' in case of HPET table
				U32 length;
				U8 revision;
				U8 checksum;
				char oemId[6];
				U64 oemTableId;
				U32 oemRevision;
				U32 creatorId;
				U32 creatorRevision;
			};

			struct __attribute__((packed)) DescriptionTable: DescriptionTableHeader {
				U8 hardware_rev_id;
				U8 comparator_count:5;
				U8 counter_size:1;
				U8 reserved:1;
				U8 legacy_replacement:1;
				uint16_t pci_vendor_id;
				AddressStructure address;
				U8 hpet_number;
				uint16_t minimum_tick;
				U8 page_protection;
			};
		}

		auto Hpet::_on_start() -> Try<> {
			return {};
		}

		auto Hpet::_on_stop() -> Try<> {
			return {};
		}

		auto Hpet::now() -> U32 {
			return 0;
		}
		auto Hpet::now64() -> U64 {
			return 0;
		}
		void Hpet::wait(U32 usecs) {
			//TODO
		}
	}
}