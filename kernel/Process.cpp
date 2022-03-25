#include "Process.hpp"

#include "Thread.hpp"
#include "ThreadCpuState.hpp"
#include "memory.hpp"
#include "scheduler.hpp"

namespace process {
	LList<Process> processes;

	Process& create_kernel(const char *name, void(*entrypoint)()) {
		auto &process = *new Process(name);

		processes.push_back(process);

		return process;
	}
}

/**/ Process::Process(const char *name):
	name(name),
	log(*this)
{}

namespace thread {
	extern LList<Thread> activeThreads;
	extern LList<Thread> sleepingThreads;
	extern LList<Thread> pausedThreads;
	extern LList<Thread> freedThreads;

	void _cleanup_thread();
}

Thread* Process::create_current_thread(memory::Page *stackPage, size_t stackSize) {
	scheduler::Guard guard;

	auto thread = new Thread(*this);
	thread->stackPage = stackPage;
	thread->storedState = (ThreadCpuState*)((size_t)stackPage->physicalAddress + stackSize - sizeof(ThreadCpuState));

	thread->state = Thread::State::active;
	thread::activeThreads.push_back(*thread);

	threads.push(*thread);

	return thread;
}

Thread* Process::create_thread(void(*entrypoint)(IpcId, void*)) {
	scheduler::Guard guard;

	auto stackPage = memory::Transaction().allocate_page();
	const auto stackSize = memory::pageSize;

	auto thread = new Thread(*this);
	thread->stackPage = stackPage;
	thread->storedState = (ThreadCpuState*)((size_t)stackPage->physicalAddress + stackSize - sizeof(ThreadCpuState));

	thread->storedState->init(entrypoint, thread::_cleanup_thread, (U8*)stackPage->physicalAddress + stackSize);

	thread->state = Thread::State::active;
	thread::activeThreads.push_back(*thread);

	threads.push(*thread);

	return thread;
}

Thread* Process::create_kernel_thread(void(*entrypoint)()) {
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