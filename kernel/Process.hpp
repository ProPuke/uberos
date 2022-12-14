#pragma once

#include <common/ipc.hpp>
#include <common/LList.hpp>
#include <common/ListUnordered.hpp>
#include <common/ipc.hpp>

#include <kernel/ProcessLog.hpp>
#include <kernel/mmu.hpp>

struct Process;

namespace process {
	auto create_kernel(const char *name, I32(*entrypoint)() = nullptr) -> Process&;
	auto get_count() -> U32;
}

namespace memory {
	struct Page;
}

struct Thread;

struct Process: LListItem<Process> {
	typedef I32(*Entrypoint)(ipc::Id, void *ipcPacket);

	/**/ Process(const char *name, Entrypoint entrypoint = nullptr);

	const char *name;
	Entrypoint entrypoint;
	ProcessLog log;

	#ifdef HAS_MMU
		mmu::MemoryMapping memoryMapping;
	#endif

	ListUnordered<Thread> threads;

	auto create_current_thread(memory::Page &stackPage, size_t stackSize) -> Thread*;

	auto create_thread(Entrypoint entrypoint, ipc::Id ipc, void *ipcPacket) -> Thread*;
	auto create_kernel_thread(I32(*entrypoint)()) -> Thread*;

	void run(ipc::Id ipc, void *ipcPacket);
};
