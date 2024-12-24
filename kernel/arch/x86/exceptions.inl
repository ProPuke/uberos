#pragma once

namespace exceptions {
	inline bool _is_active() {
		unsigned int eflags;
		asm(
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
		asm("sti");
	}

	inline void _deactivate() {
		asm("cli");
	}
}
