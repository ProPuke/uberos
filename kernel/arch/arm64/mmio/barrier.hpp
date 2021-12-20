#pragma once

namespace mmio {
	inline void barrier() {
		asm volatile("dmb sy");
	}
}
