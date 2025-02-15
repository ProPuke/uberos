#include "stdout.hpp"

#if defined(ARCH_X86_IBM_BIOS)
	#include <kernel/drivers/x86/textmode/VgaTextmode.hpp>
#elif defined(ARCH_UEFI)
	#include <kernel/drivers/uefi/textmode/UefiTextmode.hpp>
#else
	#error Unsupported platform
#endif

#include <kernel/assert.hpp>
#include <kernel/drivers.hpp>
#include <kernel/Log.hpp>

static Log log("arch::x86-ibm::stdout");

namespace arch {
	namespace x86_ibm {
		namespace stdout {
			void init() {
				// auto section = log.section("init...");

				#if defined(ARCH_X86_IBM_BIOS)
					auto device = drivers::find_and_activate<driver::textmode::VgaTextmode>();
				#elif defined(ARCH_UEFI)
					auto device = drivers::find_and_activate<driver::textmode::UefiTextmode>();
				#endif

				if(device){
					device->bind_to_console();
					device->clear();
				}
			}
		}
	}
}