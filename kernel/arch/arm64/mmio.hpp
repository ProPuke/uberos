#pragma once

#include <kernel/CriticalSection.hpp>
#include <kernel/mmio.hpp>

namespace mmio {
	void barrier();

	//TODO:periperal bus spinlock to stop other cores jumping into different peripherals at the same time

	struct PeripheralAccessGuard: NonCopyable<PeripheralAccessGuard> {
		/**/ PeripheralAccessGuard(){ CriticalSection::lock(); barrier(); };
		/**/~PeripheralAccessGuard(){ barrier(); CriticalSection::unlock(); };
	};

	struct PeripheralReadGuard: NonCopyable<PeripheralReadGuard> {
		/**/ PeripheralReadGuard(){ CriticalSection::lock(); barrier(); };
		/**/~PeripheralReadGuard(){ barrier(); CriticalSection::unlock(); };
	};

	struct PeripheralWriteGuard: NonCopyable<PeripheralWriteGuard> {
		/**/ PeripheralWriteGuard(){ CriticalSection::lock(); barrier(); };
		/**/~PeripheralWriteGuard(){ barrier(); CriticalSection::unlock(); };
	};

	inline void barrier() {
		asm volatile("dmb sy" ::: "memory");
	}
}
