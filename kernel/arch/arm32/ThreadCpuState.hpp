#pragma once

#include <common/ipc.hpp>
#include <common/types.hpp>

struct __attribute__((packed)) ThreadCpuState {
	U32 r[12];
	U32 cpsr;
	U32 lr;
	U32 pc;

	void init(I32(*entrypoint)(ipc::Id, void*), void(*cleanup)(), U8* stackEnd, ipc::Id ipc, void *ipcPacket) {
		pc = (U32)entrypoint;
		lr = (U32)cleanup;
		cpsr = 0x13 | (8<<1); //supervisor mode with irqs only

		r[0] = (U32)ipc;
		r[1] = (U32)ipcPacket;
	}

	void init_kernel(I32(*entrypoint)(), void(*cleanup)(), U8* stackEnd) {
		pc = (U32)entrypoint;
		lr = (U32)cleanup;
		cpsr = 0x13 | (8<<1); //supervisor mode with irqs only
	}
};
