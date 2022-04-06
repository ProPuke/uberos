#pragma once

struct CriticalSection {
	/**/ CriticalSection() { lock(); }
	/**/~CriticalSection() { unlock(); }

	static void lock();
	static void unlock();

	/**/ CriticalSection(const CriticalSection&) = delete;
	CriticalSection& operator=(const CriticalSection&) = delete;
};

#include "CriticalSection.inl"
