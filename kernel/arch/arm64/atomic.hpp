#pragma once

#include <common/types.hpp>

namespace arch {
	namespace arm64 {
		namespace atomic {
			static inline U32 add_return(volatile I32 *data, I32 add) {
				U32 temp;
				I32 result;

				asm volatile(
					"0:\n"
					"ldxr  %w0, %2\n"
					"add   %w0, %w0, %w3\n"
					"stlxr %w1, %w0, %2\n"
					// "cbnz  %w1, 0b"
					: "=&r" (result), "=&r" (temp), "+Q" (*data)
					: "Ir" (add)
					: "memory"
				);

				return result;
			}
		}
	}
}