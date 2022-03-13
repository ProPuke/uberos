#pragma once

#include "mmio.hpp"

#include <common/types.hpp>

namespace mmio {
	namespace arch {
		namespace raspi {
			inline void write(Address reg, U32 data) {
				return write_address((U32)reg, data);
			}
			inline U32 read(Address reg) {
				return read_address((U32)reg);
			}

			inline void write_address(U32 reg, U32 data) {
				*(volatile U32*)(U64)reg = data;
			}
			inline U32 read_address(U32 reg) {
				return *(volatile U32*)(U64)reg;
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
