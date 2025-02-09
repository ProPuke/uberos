#include "Process.hpp"

#include <kernel/memory.hpp>
#include <kernel/Thread.hpp>
#include <kernel/ThreadCpuState.hpp>

namespace process {
	LList<Process> processes;

	auto create_kernel(const char *name, void(*entrypoint)()) -> Process& {
		auto &process = *new Process(name, nullptr);

		processes.push_back(process);

		if(entrypoint){
			process.create_kernel_thread(entrypoint);
		}

		return process;
	}

	auto get_count() -> U32 {
		return processes.length();
	}
}

/**/ Process::Process(const char *name, Entrypoint entrypoint):
	name(name),
	entrypoint(entrypoint),
	log(*this)
{}

auto Process::create_current_thread(memory::Page &stackPage, size_t stackSize) -> Thread& {
	auto thread = new Thread(*this);
	thread->stackPage = &stackPage;
	thread->storedState = (ThreadCpuState*)((size_t)stackPage.physicalAddress + stackSize - sizeof(ThreadCpuState));
	thread->state = Thread::State::active;

	threads.push(thread);

	return *thread;
}

auto Process::create_thread(Entrypoint entrypoint, ipc::Id ipc, void *ipcPacket) -> Thread& {
	auto stackPage = memory::Transaction().allocate_page();
	const auto stackSize = memory::pageSize;

	auto thread = new Thread(*this);
	thread->stackPage = stackPage;
	thread->storedState = (ThreadCpuState*)((size_t)stackPage->physicalAddress + stackSize - sizeof(ThreadCpuState));

	thread->storedState->init(entrypoint, (U8*)stackPage->physicalAddress + stackSize, ipc, ipcPacket);

	thread->state = Thread::State::active;

	threads.push(thread);

	return *thread;
}

auto Process::create_kernel_thread(void(*entrypoint)()) -> Thread& {
	auto stackPage = memory::Transaction().allocate_page();
	const auto stackSize = memory::pageSize;

	auto thread = new Thread(*this);
	thread->stackPage = stackPage;
	thread->storedState = (ThreadCpuState*)((size_t)stackPage->physicalAddress + stackSize - sizeof(ThreadCpuState));
	thread->storedState->init_kernel(entrypoint, (U8*)stackPage->physicalAddress + stackSize);
	thread->state = Thread::State::active;

	threads.push(thread);

	return *thread;
}

void Process::run(ipc::Id ipc, void *ipcPacket) {
	create_thread(entrypoint, ipc, ipcPacket);
}
