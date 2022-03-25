#pragma once

#include <common/ipc.hpp>
#include <common/types.hpp>

struct __attribute__((packed)) ThreadCpuState {
	U64 x[28-18];
	U64 fp;
	U64 lr;
	U64 pc;

	void init(void(*entrypoint)(IpcId, void*), void(*cleanup)(), U8* stackEnd) {
		pc = (U64)entrypoint;
		fp = (U64)stackEnd;
		lr = (U64)cleanup;
		//TODO:set modes? (like cpsr on 32bit)
	}

	void init_kernel(void(*entrypoint)(), void(*cleanup)(), U8* stackEnd) {
		pc = (U64)entrypoint;
		fp = (U64)stackEnd;
		lr = (U64)cleanup;
		//TODO:set modes? (like cpsr on 32bit)
	}
};
