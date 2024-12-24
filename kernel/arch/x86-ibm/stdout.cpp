#include "stdout.hpp"

#include <kernel/drivers/x86/textmode/VgaTextmode.hpp>
#include <kernel/Log.hpp>

static Log log("arch::x86-ibm::stdout");

namespace arch {
	namespace x86_ibm {
		namespace stdout {
			driver::textmode::VgaTextmode device;

			void init() {
				// auto section = log.section("init...");

				drivers::install_driver(device, true);
				device.bind_to_console();

				{
					// auto bg = device.get_nearest_colour(0x000000);
					device.clear(device.consoleContext.foreground, device.consoleContext.background);
				}
			}
		}
	}
}