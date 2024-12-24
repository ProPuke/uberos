#include "framebuffer.hpp"

#include <kernel/drivers.hpp>
#include <kernel/drivers/raspi/graphics/Raspi_videocore_mailbox.hpp>
#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/log.hpp>

#include <common/format.hpp>
#include <common/stdlib.hpp>
#include <common/types.hpp>

namespace arch {
	namespace raspi {
		namespace framebuffer {
			driver::graphics::Raspi_videocore_mailbox device((U64)arch::raspi::mmio::Address::mail0_base);

			void init() {
				log::Section section("arch::raspi::framebuffer::init...");

				drivers::install_driver(device, true);
			}
		}
	}
}
