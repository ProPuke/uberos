#pragma once

#include <kernel/common.hpp>

namespace debug {
	template<typename ...Params>
	void trace(Params...);
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
	}
#else
	namespace debug {
		template<typename ...Params>
		inline void trace(Params...) {}
	}
#endif