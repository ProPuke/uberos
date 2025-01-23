#pragma once

#include "ioPort.hpp"

namespace arch {
	namespace x86 {
		typedef U16 IoPort;

		namespace ioPort {
			inline auto read8(IoPort port) -> U8 {
				U8 data;

				asm volatile(
					"in al, dx"
					: "=a" (data)
					: "d" (port)
				);

				return data;
			}

			inline auto read16(IoPort port) -> U16 {
				U16 data;

				asm volatile(
					"in ax, dx"
					: "=a" (data)
					: "d" (port)
				);

				return data;
			}

			inline auto read32(IoPort port) -> U32 {
				U32 data;

				asm volatile(
					"in eax, dx"
					: "=a" (data)
					: "d" (port)
				);

				return data;
			}

			inline void write8(IoPort port, U8 data) {
				asm volatile(
					"out dx, al"
					:
					: "d" (port), "a" (data)
				);
			}

			inline void write16(IoPort port, U16 data) {
				asm volatile(
					"out dx, ax"
					:
					: "d" (port), "a" (data)
				);
			}

			inline void write32(IoPort port, U32 data) {
				asm volatile(
					"out dx, eax"
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
