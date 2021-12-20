#pragma once

#include "stdio.hpp"
#include "exceptions.hpp"
#include <kernel/arch/arm64/atomic.hpp>

namespace spinlock {
	extern bool debug;
}

struct Spinlock {
	/**/ Spinlock(const char *name):
		name(name)
	{}

	//no accidentally copying
	/**/ Spinlock(const Spinlock&) = delete;
	Spinlock& operator=(const Spinlock&) = delete;

	void lock(const char *context, bool apply = true) {
		// exceptions::lock(apply);
		// while (__atomic_test_and_set(&_lock, __ATOMIC_ACQUIRE));
		// auto result = arch::arm64::atomic::add_return(&_lock, 69);

		U32 temp;

		asm volatile(
			"sevl\n"
			"0: wfe\n"
			"1: ldaxr %w0, [%1]\n"
			"cbnz %w0, 0b\n"
			"stxr %w0, %w2, [%1]\n"
			"cbnz %w0, 1b\n"
			: "=&r" (temp)
			: "r" (&_lock), "r" (1)
			: "memory"
		);
	}

	void unlock(bool debug = true, bool apply = true) {
		// // if(debug) stdio::print("unlock! ", this->name, "\n");
		// __atomic_clear(&_lock, __ATOMIC_RELEASE);
		// // exceptions::unlock(apply);
		// _lock = 0;

		asm volatile(
			"stlr %w1, [%0]\n"
			:
			: "r" (&_lock), "r" (0)
			: "memory"
		);
	}

	const char *name;

	// private: int _lock = 0;
	// volatile int _lock = 0;
	volatile U32 _lock = 0;
};

struct Spinlock_Guard {
	/**/ Spinlock_Guard(Spinlock &lock, const char *context, bool debug = true):
		context(context),
		debug(debug),
		_lock(lock)
	{
		// if(spinlock::debug&&context) stdio::print("lock ", &_lock, " (", lock._lock, ") ", context, "{\n");
		_lock.lock(context);
		// if(spinlock::debug&&context) stdio::print("} lock ", &_lock, " ", context, "\n");
	}
	
	/**/~Spinlock_Guard(){
		// if(spinlock::debug&&context) stdio::print("unlock ", &_lock, " ", context, "{\n");
		_lock.unlock(debug);
		// if(spinlock::debug&&context) stdio::print("} unlock ", &_lock, " ", context, "\n");
	}

	const char *context;
	bool debug;

	private: Spinlock &_lock;
};
