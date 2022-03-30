#pragma once

#include <common/ipc.hpp>
#include <common/types.hpp>

struct __attribute__((packed)) ThreadCpuState {
	U64 x[28-18];
	U64 fp;
	U64 lr;
	U64 pc;

	void init(I32(*entrypoint)(ipc::Id, void*), void(*cleanup)(), U8* stackEnd, ipc::Id ipc, void *ipcPacket) {
		pc = (U64)entrypoint;
		fp = (U64)stackEnd;
		lr = (U64)cleanup;
		//TODO:set modes? (like cpsr on 32bit)

		x[0] = (U32)ipc;
		x[1] = (U64)ipcPacket;
	}

	void init_kernel(I32(*entrypoint)(), void(*cleanup)(), U8* stackEnd) {
		pc = (U64)entrypoint;
		fp = (U64)stackEnd;
		lr = (U64)cleanup;
		//TODO:set modes? (like cpsr on 32bit)
	}
};
