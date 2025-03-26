#pragma once

#include "processor.hpp"

namespace processor {
	__attribute__((always_inline)) inline void pause() {
		asm volatile("pause");
	}
}
