#pragma once

#include <common/Try.hpp>
#include <common/types.hpp>

namespace exceptions {
	namespace interrupt {
		typedef const void* (*Subscriber)(void *cpu);

		void subscribe(U8, Subscriber callback);
		void unsubscribe(U8, Subscriber callback);
		void subscribe_all(Subscriber callback);
		void unsubscribe_all(Subscriber callback);
	}

	namespace irq {
		typedef void (*Subscriber)(U8 irq, void*);

		auto subscribe(U8, Subscriber callback, void *data) -> Try<>;
		void unsubscribe(U8, Subscriber callback, void *data);
	}

	void enable();
	void disable();

	void lock();
	void unlock();

	void init();

	struct Guard: NonCopyable<Guard> {
		/**/ Guard() { lock(); }
		/**/~Guard() { unlock(); }
	};

	void _on_irq(U8);
	void _activate();
	void _deactivate();
	bool _is_active();
}

#include "exceptions.inl"

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/exceptions.inl>
#endif

#if defined(ARCH_ARM64)
	#include <kernel/arch/arm64/exceptions.inl>
#endif

#if defined(ARCH_X86)
	#include <kernel/arch/x86/exceptions.inl>
#endif
