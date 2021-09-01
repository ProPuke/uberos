#pragma once

namespace exceptions {
	void lock(bool apply=true);
	void unlock(bool apply=true);

	struct Guard {
		/**/ Guard() { lock(); }
		/**/~Guard() { unlock(); }

		/**/ Guard(const Guard&) = delete;
		Guard& operator=(const Guard&) = delete;
	};

	void _activate();
	void _deactivate();
	bool _is_active();
}

#include "exceptions.inl"

#if defined(ARCH_ARM32)
	#include "arch/arm32/exceptions.inl"
#endif

#if defined(ARCH_ARM64)
	#include "arch/arm64/exceptions.inl"
#endif
