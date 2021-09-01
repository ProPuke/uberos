#include "usb.hpp"

#include "mmio.hpp"
#include <kernel/stdio.hpp>

namespace arch {
	namespace raspi {
		namespace usb {
			void init(bool plugAndPlay) {
				stdio::Section section("arch::raspi::usb::init...");

				auto userId = mmio::read(mmio::Address::usb_core_user_id);
				auto vendorId = mmio::read(mmio::Address::usb_core_vendor_id);

				const auto synopsys_id = 0x4F54280A;

				switch(vendorId){
					case synopsys_id:
						stdio::print_info("Synopsys vendor detected");
					break;
					default:
						stdio::print_info("Error: Unrecognised vendor id (", ::to_string_hex(vendorId), ')');
				}

				stdio::print_info("User Id: ", ::to_string_hex(userId));
			}
		}
	}
}
