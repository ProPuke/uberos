#pragma once

#include <common/ipc.hpp>
#include <common/types.hpp>

struct __attribute__((packed)) ThreadCpuState {
	U32 eip;
	U32 esp;
	U32 edi;
	U32 esi;
	U32 ebx;
	U32 ebp;


	void init(void(*entrypoint)(ipc::Id, void*), U8* stackEnd, ipc::Id ipc, void *ipcPacket) {
		eip = (U64)entrypoint;
		esp = (U64)stackEnd;

		// x[0] = (U32)ipc;
		// x[1] = (U64)ipcPacket;
	}

	void init_kernel(void(*entrypoint)(), U8* stackEnd) {
		eip = (U64)entrypoint;
		esp = (U64)stackEnd;
	}
};
