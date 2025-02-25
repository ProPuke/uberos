#include "serial.hpp"

#include <drivers/hosted/serial/Stdout.hpp>

#include <kernel/assert.hpp>
#include <kernel/drivers.hpp>
#include <kernel/Log.hpp>

static Log log("arch::hosted::serial");

namespace arch {
	namespace hosted {
		namespace serial {
			void init() {
				auto section = log.section("init...");

				auto device = drivers::find_and_activate<driver::serial::Stdout>();
				assert(device);

				device->bind_to_console();
			}
		}
	}
}