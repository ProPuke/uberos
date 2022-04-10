#pragma once

#include <kernel/CriticalSection.hpp>
#include <kernel/mmio.hpp>

namespace mmio {
	void barrier();

	struct PeripheralAccessGuard {
		/**/ PeripheralAccessGuard(){ CriticalSection::lock(); barrier(); };
		/**/~PeripheralAccessGuard(){ barrier(); CriticalSection::unlock(); };

		/**/ PeripheralAccessGuard(const PeripheralAccessGuard&) = delete;
		PeripheralAccessGuard& operator=(const PeripheralAccessGuard&) = delete;
	};

	struct PeripheralReadGuard {
		/**/ PeripheralReadGuard(){ CriticalSection::lock(); barrier(); };
		/**/~PeripheralReadGuard(){ barrier(); CriticalSection::unlock(); };

		/**/ PeripheralReadGuard(const PeripheralReadGuard&) = delete;
		PeripheralReadGuard& operator=(const PeripheralReadGuard&) = delete;
	};

	struct PeripheralWriteGuard {
		/**/ PeripheralWriteGuard(){ CriticalSection::lock(); barrier(); };
		/**/~PeripheralWriteGuard(){ barrier(); CriticalSection::unlock(); };

		/**/ PeripheralWriteGuard(const PeripheralWriteGuard&) = delete;
		PeripheralWriteGuard& operator=(const PeripheralWriteGuard&) = delete;
	};

	inline void barrier() {
		asm volatile("dmb sy" ::: "memory");
	}
}
