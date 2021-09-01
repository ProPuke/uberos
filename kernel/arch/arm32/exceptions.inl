#pragma once

#include "exceptions.hpp"

namespace exceptions {
	inline bool _is_active() {
		int res;
		asm volatile(
			"mrs %0, CPSR"
			
			: "=r" (res)
		);
		return ((res >> 7) & 1) == 0;
	}

	inline void _activate() {
		asm volatile("cpsie i");
	}

	inline void _deactivate() {
		asm volatile("cpsid i");
	}
}
