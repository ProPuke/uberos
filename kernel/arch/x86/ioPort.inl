#pragma once

#include "ioPort.hpp"

namespace arch {
	namespace x86 {
		typedef U16 IoPort;

		namespace ioPort {
			inline auto read8(IoPort port) -> U8 {
				U8 data;

				asm(
					"in al, dx"
					: "=a" (data)
					: "d" (port)
				);

				return data;
			}

			inline auto read16(IoPort port) -> U16 {
				U16 data;

				asm(
					"in ax, dx"
					: "=a" (data)
					: "d" (port)
				);

				return data;
			}

			inline void write8(IoPort port, U8 data) {
				asm(
					"out dx, al"
					:
					: "d" (port), "a" (data)
				);
			}

			inline void write16(IoPort port, U16 data) {
				asm(
					"out dx, ax"
					:
					: "d" (port), "a" (data)
				);
			}

			inline void wait() {
				// write to an unused port (causes a few microsecond delay)
				write8(0x80, 0); //bios post code
			}
		}
	}
}
