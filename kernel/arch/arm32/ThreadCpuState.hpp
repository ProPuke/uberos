#pragma once

#include <common/ipc.hpp>
#include <common/types.hpp>

struct __attribute__((packed)) ThreadCpuState {
	U32 r[12];
	U32 cpsr;
	U32 lr;
	U32 pc;

	void init(void(*entrypoint)(IpcId, void*), void(*cleanup)(), U8* stackEnd) {
		pc = (U32)entrypoint;
		lr = (U32)cleanup;
		cpsr = 0x13 | (8<<1); //supervisor mode with irqs only
	}

	void init_kernel(void(*entrypoint)(), void(*cleanup)(), U8* stackEnd) {
		pc = (U32)entrypoint;
		lr = (U32)cleanup;
		cpsr = 0x13 | (8<<1); //supervisor mode with irqs only
	}
};
