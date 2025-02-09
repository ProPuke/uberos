#pragma once

#include <kernel/drivers/Scheduler.hpp>
#include <kernel/Process.hpp>

#include <common/EventEmitter.hpp>
#include <common/LList.hpp>
#include <common/types.hpp>

#include <atomic>

namespace arch {
	namespace arm {
		namespace scheduler {
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

	U32 get_total_count();
	U32 get_active_count();
}

struct ThreadCpuState;

struct Thread: LListItem<Thread> {
	// note that this really stores the frozen moment in time in the kernel 

	friend void arch::arm::scheduler::init();
	friend auto Process::create_thread(Process::Entrypoint entrypoint, ipc::Id ipc, void *ipcPacket) -> Thread&;
	friend auto Process::create_kernel_thread(void(*entrypoint)()) -> Thread&;
	friend auto Process::create_current_thread(memory::Page &stackPage, size_t stackSize) -> Thread&;

	private:
		/**/ Thread(Process &process);
		/**/~Thread();
	public:

	// /**/ Thread(void *stack, U32 stackSize, void(*entrypoint()));

	DriverReference<driver::Scheduler> scheduler;

	ThreadCpuState *storedState = nullptr;
	memory::Page *stackPage = nullptr;
	Process &process;
	U16 priority = 100; // multiplied by process priority

	struct Event {
		enum struct Type {
			terminated
		} type;

		union {
			struct {

			} terminated;
		};
	};

	EventEmitter<Event> events;

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

	void sleep(U32 usecs);

	void pause();
	void resume();
	void terminate();

	void set_priority(U16 priority);

	U32 _pending_timer_id = 0;
};

#include <common/stdlib.hpp>

template<> inline auto to_string(Thread::State state) -> const char* {
	return state<=Thread::max_state?Thread::state_name[state]:to_string((U8)state);
}

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/Thread.inl>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/Thread.inl>
#elif defined(ARCH_X86)
	#include <kernel/arch/x86/Thread.inl>
#endif
