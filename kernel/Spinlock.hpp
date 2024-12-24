#pragma once

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/Spinlock.hpp>
	using arch::arm32::Spinlock;
	
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/Spinlock.hpp>
	using arch::arm64::Spinlock;

#elif defined(ARCH_X86)
	#include <kernel/arch/x86/Spinlock.hpp>
	using arch::x86::Spinlock;
#endif

struct Spinlock_Guard {
	/**/ Spinlock_Guard(Spinlock &lock, const char *context = "unnamed"):
		context(context),
		_lock(lock)
	{
		_lock.lock(context);
	}
	
	/**/~Spinlock_Guard(){
		_lock.unlock();
	}

	const char *context;

	private: Spinlock &_lock;
};
