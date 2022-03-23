#pragma once

#include <common/types.hpp>

namespace mmio {
	void write32(U32 reg, U32 data);
	auto read32(U32 reg) -> U32;
	
	void delay(I32 count);
}

#include "mmio.inl"
