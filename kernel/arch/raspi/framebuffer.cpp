#include "framebuffer.hpp"

#include <common/format.hpp>
#include <common/stdlib.hpp>
#include <common/types.hpp>

#include <kernel/deviceManager.hpp>
#include <kernel/driver/graphics/Raspi_videocore_mailbox.hpp>
#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/stdio.hpp>

namespace framebuffer {
	namespace arch {
		namespace raspi {
			driver::graphics::Raspi_videocore_mailbox device((U64)mmio::arch::raspi::Address::mail0_base);

			void init() {
				stdio::Section section("framebuffer::arch::raspi::init...");

				deviceManager::add_device(device);
			}
		}
	}
}
