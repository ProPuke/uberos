#include "serial.hpp"

#include <kernel/drivers/hosted/serial/Stdout.hpp>

namespace arch {
	namespace hosted {
		namespace serial {
			driver::serial::Stdout device;

			void init() {
				log::Section section("arch::hosted::serial::init...");

				drivers::install_driver(device, true);
				device.bind_to_console();
			}
		}
	}
}