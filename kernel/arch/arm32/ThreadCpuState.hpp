#pragma once

#include <common/types.hpp>

struct __attribute__((packed)) ThreadCpuState {
	U32 r[12-4];
	U32 cpsr;
	U32 lr;
	U32 pc;

	void init_kernel(void(*entrypoint)(), void(*cleanup)(), U8* stackEnd) {
		pc = (U32)entrypoint;
		lr = (U32)cleanup;
		cpsr = 0x13 | (8<<1); //supervisor mode with irqs only
	}
};
