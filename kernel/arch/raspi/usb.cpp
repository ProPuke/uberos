#include "usb.hpp"

#include "mmio.hpp"

#include <common/format.hpp>

#include <kernel/stdio.hpp>

namespace mmio {
	using namespace arch::raspi;
}

namespace usb {
	namespace arch {
		namespace raspi {
			void init(bool plugAndPlay) {
				stdio::Section section("usb::arch::raspi::init...");

				auto userId = mmio::read_address(mmio::Address::usb_core_user_id);
				auto vendorId = mmio::read_address(mmio::Address::usb_core_vendor_id);

				const auto synopsys_id = 0x4F54280A;

				switch(vendorId){
					case synopsys_id:
						stdio::print_info("Synopsys vendor detected");
					break;
					default:
						stdio::print_info("Error: Unrecognised vendor id (", format::Hex32{vendorId}, ')');
				}

				stdio::print_info("User Id: ", format::Hex32{userId});
			}
		}
	}
}
