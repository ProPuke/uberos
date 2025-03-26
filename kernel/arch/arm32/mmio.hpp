#pragma once

#include <kernel/CriticalSection.hpp>
#include <kernel/mmio.hpp>

namespace mmio {
	void barrier();

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
		asm volatile(
			"mov r12, #0\n"
			"mcr p15, 0, r12, c7, c10, 5\n"
		);
	}
}
