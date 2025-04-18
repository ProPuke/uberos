#pragma once

#include <kernel/CriticalSection.hpp>

#include <atomic>

enum struct LockType {
	flat, // no nesting allowed - single depth
	recursive // nested locking allowed within the same processor
};

template <LockType lockType>
struct Lock;

template <typename Type>
struct Lock_Guard {
	/**/ Lock_Guard(Type &lock, const char *context = "unnamed"):
		context(context),
		_lock(lock)
	{
		_lock.lock();
	}
	
	/**/~Lock_Guard(){
		_lock.unlock();
	}

	const char *context;

private:
	Type &_lock;
};

template <>
struct Lock<LockType::flat>: NonCopyable<Lock<LockType::flat>> {
	constexpr /**/ Lock(const char *name = "unnamed"):
		name(name)
	{}

	void lock();
	void unlock();

	const char *name;

protected:

	#ifdef HAS_SMP
		std::atomic<U32> lockActive = 0;
	#endif
};

template <>
struct Lock<LockType::recursive>: NonCopyable<Lock<LockType::recursive>> {
	constexpr /**/ Lock(const char *name = "unnamed"):
		name(name)
	{}

	void lock();
	void unlock();

	const char *name;

protected:

	#ifdef HAS_SMP
		std::atomic<U32> lockDepth = 0;
		std::atomic<U32> lockProcessor = ~0;
	#endif
};

#include "Lock.inl"
