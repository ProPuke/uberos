#include "framebuffer.hpp"

#include <kernel/drivers.hpp>
#include <kernel/drivers/x86/graphics/VBE.hpp>
#include <kernel/Log.hpp>

#include <common/format.hpp>
#include <common/stdlib.hpp>
#include <common/types.hpp>

static Log log("arch::x86-ibm-bios::framebuffer");

namespace arch {
	namespace x86_ibm_bios {
		namespace framebuffer {
			driver::graphics::VBE device;

			void init() {
				auto section = log.section("init...");

				drivers::install_driver(device, true);
			}
		}
	}
}
