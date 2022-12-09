#include "usb.hpp"

#include "mmio.hpp"

#include <common/format.hpp>

#include <kernel/log.hpp>

namespace mmio {
	using namespace arch::raspi;
}

namespace usb {
	namespace arch {
		namespace raspi {
			void init(bool plugAndPlay) {
				log::Section section("usb::arch::raspi::init...");

				auto userId = mmio::read_address(mmio::Address::usb_core_user_id);
				auto vendorId = mmio::read_address(mmio::Address::usb_core_vendor_id);

				const auto synopsys_id = 0x4F54280A;

				switch(vendorId){
					case synopsys_id:
						log::print_info("Synopsys vendor detected");
					break;
					default:
						log::print_info("Error: Unrecognised vendor id (", format::Hex32{vendorId}, ')');
				}

				log::print_info("User Id: ", format::Hex32{userId});
			}
		}
	}
}
