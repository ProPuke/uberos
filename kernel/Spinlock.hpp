#pragma once

#ifdef ARCH_ARM64
	#include "arch/arm64/Spinlock.hpp"

	using arch::arm64::Spinlock;
#endif

struct Spinlock_Guard {
	/**/ Spinlock_Guard(Spinlock &lock, const char *context, bool debug = true):
		context(context),
		debug(debug),
		_lock(lock)
	{
		_lock.lock(context);
	}
	
	/**/~Spinlock_Guard(){
		_lock.unlock(debug);
	}

	const char *context;
	bool debug;

	private: Spinlock &_lock;
};
