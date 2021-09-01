#pragma once

#include "mmio.hpp"

#include <common/types.hpp>

namespace arch {
	namespace raspi {
		namespace mmio {
			inline void write(Address reg, U32 data) {
				*(volatile U32*)reg = data;
			}

			inline U32 read(Address reg) {
				return *(volatile U32*)reg;
			}

			// Loop <delay> times in a way that the compiler won't optimize away
			inline void delay(I32 count) {
				asm volatile(
					"__delay_%=: subs %[count], %[count], #1\n"
					"bne __delay_%=\n"
					: "=r"(count)
					: [count] "0"(count)
					: "cc"
				);
			}
		}
	}
}
