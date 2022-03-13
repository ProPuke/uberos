#pragma once

#include "ProcessLog.hpp"
#include "mmu.hpp"
#include <common/LList.hpp>
#include <common/ListUnordered.hpp>

struct Process;

namespace process {
	Process& create_kernel(const char *name, void(*entrypoint)());
}

struct Thread;

struct Process: LListItem<Process> {
	/**/ Process(const char *name);

	const char *name;
	ProcessLog log;

	#ifdef HAS_MMU
		mmu::MemoryMapping memoryMapping;
	#endif

	ListUnordered<Thread> threads;

	Thread* create_thread(void(*entrypoint)());
};
