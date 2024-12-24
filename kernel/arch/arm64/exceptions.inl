#pragma once

// #include <kernel/log.hpp>

namespace exceptions {
	inline bool _is_active() {
		int res;
		asm volatile(
			"mrs %0, daif"
			
			: "=r" (res)
		);
		return (res & 2) == 0;
	}

	inline int _get_level() {
		int res;
		asm volatile(
			"mrs %0, CurrentEL\n"
			"lsr %0, %0, #2"
			
			: "=r" (res)
		);
		return res;
	}

	inline void _activate() {
		// log::print_info("exception level ", _get_level());
		asm volatile(
			"msr daifclr, #0b0010\n"
		);
	}

	inline void _deactivate() {
		asm volatile(
			"msr daifset, #0b0010\n"
		);
	}
}
