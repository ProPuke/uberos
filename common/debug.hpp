#pragma once

#ifdef KERNEL
	#include <kernel/exceptions.hpp>
	#include <kernel/panic.hpp>
#endif

namespace debug {
	void assert(bool);
	template<typename Type>
	inline auto assert_return(Type value) -> Type { assert(value); return value; }

	void halt();
}

#ifdef DEBUG
	namespace debug {
		inline void assert(bool test) { if(!test) halt(); }
		// inline void halt() { ::halt(); }
		#ifdef KERNEL
			inline void halt() { panic::panic().print_details("DEBUG HALT").print_stacktrace(); }
		#else
			inline void halt() { asm volatile("hlt"); }
		#endif
	}
#else
	namespace debug {
		inline void assert(bool) {}
		inline void halt() {}
	}
#endif
