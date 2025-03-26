#pragma once

struct CriticalSection: NonCopyable<CriticalSection> {
	/**/ CriticalSection() { lock(); }
	/**/~CriticalSection() { unlock(); }

	static void lock();
	static void unlock();
};

#include "CriticalSection.inl"
