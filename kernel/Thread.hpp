#pragma once

#include "Process.hpp"
#include <common/types.hpp>
#include <common/LList.hpp>
#include <common/Callback.hpp>
#include <atomic>

namespace scheduler {
	namespace arch {
		namespace arm {
			void init();
		}
	}
}

namespace memory {
	struct Page;
}

struct Thread;

namespace thread {
	extern std::atomic<Thread*> currentThread;
}

struct ThreadCpuState;

struct Thread: LListItem<Thread> {
	// note that this really stores the frozen moment in time in the kernel 

	friend void scheduler::arch::arm::init();
	friend Thread* Process::create_thread(void(*entrypoint)());
	friend Thread* Process::create_current_thread(memory::Page *stackPage, size_t stackSize);

	private: /**/ Thread(Process &process);
	public:

	// /**/ Thread(void *stack, U32 stackSize, void(*entrypoint()));

	ThreadCpuState *storedState = nullptr;
	memory::Page *stackPage = nullptr;
	Process &process;

	LList<Callback> on_deleted;

	static void swap_state(Thread &from, Thread &to);

	enum State: U8 {
		active,
		sleeping,
		paused,
		terminated
	} state = State::paused;

	static constexpr auto max_state = State::terminated;

	static constexpr const char *state_name[max_state+1] = {
		"active",
		"sleeping",
		"paused",
		"terminated"
	};

	U64 sleep_wake_time = 0;

	void sleep(U64 usecs);

	void pause();
	void resume();
	void terminate();
};

#include <common/stdlib.hpp>

inline const char* to_string(Thread::State state) {
	return state<=Thread::max_state?Thread::state_name[state]:to_string((U8)state);
}

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/Thread.inl>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/Thread.inl>
#endif
