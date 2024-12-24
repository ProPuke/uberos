#pragma once

#include <kernel/common.hpp>

namespace debug {
	template<typename ...Params>
	void trace(Params...);
	void assert(bool);
	void halt();
}

#ifdef DEBUG
	#ifdef KERNEL
		#include <kernel/logging.hpp>
		#include <kernel/exceptions.hpp>
	#else
		#include <common/log.hpp>
	#endif

	namespace debug {
		template<typename ...Params>
		#ifdef KERNEL
			inline void trace(Params ...params) { logging::print_debug(params...); }
		#else
			inline void trace(Params ...params) { log::print_debug(params...); }
		#endif
		inline void assert(bool test) { if(!test) halt(); }
		inline void halt() { ::halt(); }
		// #ifdef KERNEL
		// 	inline void halt() { interrupts::_deactivate(); asm("hlt"); }
		// #else
		// 	inline void halt() { asm("hlt"); }
		// #endif
	}
#else
	namespace debug {
		template<typename ...Params>
		inline void trace(Params...) {}
		inline void assert(bool) {}
		inline void halt() {}
	}
#endif