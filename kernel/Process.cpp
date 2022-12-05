#include "Process.hpp"

#include "Thread.hpp"
#include "ThreadCpuState.hpp"
#include "memory.hpp"
#include "scheduler.hpp"

namespace process {
	LList<Process> processes;

	auto create_kernel(const char *name, void(*entrypoint)()) -> Process& {
		auto &process = *new Process(name, nullptr);

		processes.push_back(process);

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

namespace thread {
	extern LList<Thread> activeThreads;
	extern LList<Thread> sleepingThreads;
	extern LList<Thread> pausedThreads;
	extern LList<Thread> freedThreads;

	void _cleanup_thread();
}

Thread* Process::create_current_thread(memory::Page &stackPage, size_t stackSize) {
	scheduler::Guard guard;

	auto thread = new Thread(*this);
	thread->stackPage = &stackPage;
	thread->storedState = (ThreadCpuState*)((size_t)stackPage.physicalAddress + stackSize - sizeof(ThreadCpuState));

	thread->state = Thread::State::active;
	thread::activeThreads.push_back(*thread);

	threads.push(*thread);

	return thread;
}

Thread* Process::create_thread(Entrypoint entrypoint, ipc::Id ipc, void *ipcPacket) {
	scheduler::Guard guard;

	auto stackPage = memory::Transaction().allocate_page();
	const auto stackSize = memory::pageSize;

	auto thread = new Thread(*this);
	thread->stackPage = stackPage;
	thread->storedState = (ThreadCpuState*)((size_t)stackPage->physicalAddress + stackSize - sizeof(ThreadCpuState));

	thread->storedState->init(entrypoint, thread::_cleanup_thread, (U8*)stackPage->physicalAddress + stackSize, ipc, ipcPacket);

	thread->state = Thread::State::active;
	thread::activeThreads.push_back(*thread);

	threads.push(*thread);

	return thread;
}

Thread* Process::create_kernel_thread(I32(*entrypoint)()) {
	scheduler::Guard guard;

	auto stackPage = memory::Transaction().allocate_page();
	const auto stackSize = memory::pageSize;

	auto thread = new Thread(*this);
	thread->stackPage = stackPage;
	thread->storedState = (ThreadCpuState*)((size_t)stackPage->physicalAddress + stackSize - sizeof(ThreadCpuState));

	thread->storedState->init_kernel(entrypoint, thread::_cleanup_thread, (U8*)stackPage->physicalAddress + stackSize);

	thread->state = Thread::State::active;
	thread::activeThreads.push_back(*thread);

	threads.push(*thread);

	return thread;
}

void Process::run(ipc::Id ipc, void *ipcPacket) {
	create_thread(entrypoint, ipc, ipcPacket);
}
