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
	mmu::MemoryMapping memoryMapping;

	ListUnordered<Thread> threads;

	Thread* create_thread(void(*entrypoint)());
};
