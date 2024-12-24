#include "nmi.hpp"

#include <kernel/arch/x86/ioPort.hpp>

namespace arch {
	namespace x86 {
		namespace nmi {
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
