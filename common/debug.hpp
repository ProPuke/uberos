#pragma once

#ifdef KERNEL
	#include <kernel/exceptions.hpp>
#endif

namespace debug {
	void assert(bool);
	void halt();
}

#ifdef DEBUG
	namespace debug {
		inline void assert(bool test) { if(!test) halt(); }
		// inline void halt() { ::halt(); }
		#ifdef KERNEL
			inline void halt() { exceptions::_deactivate(); asm volatile("hlt"); }
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