#pragma once

#include <common/types.hpp>

namespace mmio {
	void write32(U32 reg, U32 data);
	U32 read32(U32 reg);
	
	void delay(I32 count);
}

#include "mmio.inl"
