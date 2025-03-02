#pragma once

#include "processor.hpp"

namespace processor {
	__attribute__((always_inline)) inline auto get_current_id() -> U32 {
		//TODO: organise this in a dedicated apic module?
		const auto lapic_base = 0xfee00000;
		const auto lapic_id = 0x20;
		return *(volatile U32*)(lapic_base + lapic_id) >> 24;
	}

	__attribute__((always_inline)) inline void pause() {
		asm volatile("pause");
	}
}
