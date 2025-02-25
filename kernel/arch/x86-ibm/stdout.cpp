#include "stdout.hpp"

#if defined(ARCH_X86_IBM_BIOS)
	#include <drivers/x86/textmode/VgaTextmode.hpp>
#elif defined(ARCH_UEFI)
	#include <drivers/uefi/textmode/UefiTextmode.hpp>
#else
	#error Unsupported platform
#endif

#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/assert.hpp>
#include <kernel/drivers.hpp>
#include <kernel/Log.hpp>

static Log log("arch::x86-ibm::stdout");

namespace arch {
	namespace x86_ibm {
		namespace stdout {
			void init() {
				// auto section = log.section("init...");

				// x86::ioPort::write8(0x3d4, 0x0f);
				// x86::ioPort::write8(0x3d5, 0x00);
				// x86::ioPort::write8(0x3d4, 0x0e);
				// x86::ioPort::write8(0x3d5, 0x00);

				// // Enable 8x8 character font (switching to 80x50 mode)
				// x86::ioPort::write8(0x3D4, 0x09);  
				// x86::ioPort::write8(0x3D5, 0x07);  // Set character cell height to 8 pixels

				// // Set maximum scan line to 7 (forces 8-pixel character height)
				// x86::ioPort::write8(0x3D4, 0x12);
				// x86::ioPort::write8(0x3D5, 0x28);

				// // Adjust the row count for text mode (CRTC register 0x11)
				// x86::ioPort::write8(0x3D4, 0x11);
				// auto crtc_val = x86::ioPort::read8(0x3D5);
				// x86::ioPort::write8(0x3D5, crtc_val & ~0x80);  // Unlock CRTC registers

				// // Set the number of rows to 50 (this depends on font height)
				// x86::ioPort::write8(0x3D4, 0x12);
				// x86::ioPort::write8(0x3D5, 0x27);

				// // Re-enable protection
				// x86::ioPort::write8(0x3D4, 0x11);
				// x86::ioPort::write8(0x3D5, crtc_val | 0x80);

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