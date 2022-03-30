#pragma once

#if defined(ARCH_ARM32)
	#include "arch/arm32/Spinlock.hpp"
	using arch::arm32::Spinlock;
	
#elif defined(ARCH_ARM64)
	#include "arch/arm64/Spinlock.hpp"
	using arch::arm64::Spinlock;
#endif

template <bool lock_scheduler = true, bool lock_exceptions = true>
struct Spinlock_Guard {
	/**/ Spinlock_Guard(Spinlock<lock_scheduler, lock_exceptions> &lock, const char *context = "unnamed"):
		context(context),
		_lock(lock)
	{
		_lock.lock(context);
	}
	
	/**/~Spinlock_Guard(){
		_lock.unlock();
	}

	const char *context;

	private: Spinlock<lock_scheduler, lock_exceptions> &_lock;
};
