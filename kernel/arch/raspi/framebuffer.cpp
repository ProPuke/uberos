#include "framebuffer.hpp"

#include <common/format.hpp>
#include <common/stdlib.hpp>
#include <common/types.hpp>

#include <kernel/deviceManager.hpp>
#include <kernel/driver/graphics/Rpi_videocore_mailbox.hpp>
#include <kernel/stdio.hpp>

namespace framebuffer {
	namespace arch {
		namespace raspi {
			driver::graphics::Rpi_videocore_mailbox device;

			void init() {
				stdio::Section section("framebuffer::arch::raspi::init...");

				deviceManager::add_device(device);
			}
		}
	}
}
