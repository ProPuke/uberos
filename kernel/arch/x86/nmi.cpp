#include "nmi.hpp"

#include <kernel/arch/x86/ioPort.hpp>

namespace arch {
	namespace x86 {
		namespace nmi {
			// void _on_start() {
			// 	// TRY(api.subscribe_ioPort(pic1Command));
			// 	// TRY(api.subscribe_ioPort(pic1Data));
			// 	// TRY(api.subscribe_ioPort(pic2Command));
			// 	// TRY(api.subscribe_ioPort(pic2Data));
			// }

			bool enabled = true;

			void enable() {
				ioPort::write8(0x70, ioPort::read8(0x70) & 0x7F);
				ioPort::read8(0x71);
				enabled = true;
			}

			void disable() {
				ioPort::write8(0x70, ioPort::read8(0x70) | 0x80);
				ioPort::read8(0x71);
				enabled = false;
			}

			auto isEnabled() -> bool {
				return enabled;
			}
		}
	}
}
