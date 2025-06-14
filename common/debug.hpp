#pragma once

#ifdef KERNEL
	#include <kernel/exceptions.hpp>
	#include <kernel/panic.hpp>
#endif

namespace debug {
	template <typename Type>
	auto assert(Type test) -> Type;
	template<typename Type>
	inline auto assert_return(Type value) -> Type { assert(value); return value; }

	void halt();
}

#ifdef DEBUG
	namespace debug {
		template <typename Type>
		inline auto assert(Type test) -> Type { if(!test) halt(); return test; }
		// inline void halt() { ::halt(); }
		#ifdef KERNEL
			inline void halt() { panic::panic().print_details("DEBUG HALT").print_stacktrace(); }
		#else
			inline void halt() { asm volatile("hlt"); }
		#endif
	}
#else
	namespace debug {
		template <typename Type>
		inline auto assert(Type test) -> Type { return test; }
		inline void halt() {}
	}
#endif
