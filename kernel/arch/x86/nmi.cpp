#include "nmi.hpp"

#include <kernel/arch/x86/ioPort.hpp>

namespace arch {
	namespace x86 {
		namespace nmi {
			// void _on_start() {
			// 	// if(!api.subscribe_ioPort(pic1Command)||!api.subscribe_ioPort(pic1Data)||!api.subscribe_ioPort(pic2Command)||!api.subscribe_ioPort(pic2Data)) return {"I/O ports not available"};
			// }

			void enable() {
				ioPort::write8(0x70, ioPort::read8(0x70) & 0x7F);
				ioPort::read8(0x71);
			}

			void disable() {
				ioPort::write8(0x70, ioPort::read8(0x70) | 0x80);
				ioPort::read8(0x71);
			}
		}
	}
}
