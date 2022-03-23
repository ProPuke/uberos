#pragma once

#include "mmio.hpp"

namespace mmio {
	inline void write32(U32 reg, U32 data) {
		*(volatile U32*)(U64)reg = data;
	}
	inline auto read32(U32 reg) -> U32 {
		return *(volatile U32*)(U64)reg;
	}

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
