#pragma once

#include <common/types.hpp>

namespace mmio {
	auto read32(U32 reg) -> U32;
	auto read64(U32 reg) -> U64;

	void write32(U32 reg, U32 data);
	void write64(U32 reg, U64 data);
	
	void delay(I32 count);
}

#include "mmio.inl"

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/mmio.hpp>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/mmio.hpp>
#endif