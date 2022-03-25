#pragma once

#include <common/ipc.hpp>
#include <common/LList.hpp>
#include <common/ListUnordered.hpp>

#include <kernel/ProcessLog.hpp>
#include <kernel/mmu.hpp>

#include <cstddef>

struct Process;

namespace process {
	Process& create_kernel(const char *name, void(*entrypoint)());
}

namespace memory {
	struct Page;
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

	auto create_current_thread(memory::Page *stackPage, size_t stackSize) -> Thread*;

	auto create_thread(void(*entrypoint)(IpcId, void* ipcData)) -> Thread*;
	auto create_kernel_thread(void(*entrypoint)()) -> Thread*;
};
