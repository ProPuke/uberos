#pragma once

namespace exceptions {
	inline bool _is_active() {
		unsigned int eflags;
		asm volatile(
			"pushf\n"
			"pop eax\n"
			"mov %0, eax\n"
			: "=r"(eflags)
			:
			: "eax", "memory"
		);
		return eflags & 0x200;
	}

	inline void _activate() {
		asm volatile("sti");
	}

	inline void _deactivate() {
		asm volatile("cli");
	}
}
