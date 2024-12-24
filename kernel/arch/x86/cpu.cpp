#include "cpu.hpp"

#include <kernel/drivers.hpp>
#include <kernel/drivers/x86/processor/X86.hpp>
#include <kernel/Log.hpp>

static Log log("arch::x86::cpu");

namespace arch {
	namespace x86 {
		namespace cpu {
			driver::processor::X86 device;

			void init() {
				auto section = log.section("init");

				drivers::install_driver(device, true);
			}
		}
	}
}
