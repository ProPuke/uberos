#pragma once

#ifndef DEBUG
	template<typename ...Params>
	inline void assert(bool assertion, Params ...messageParams) {}

#else
	#include "log.hpp"

	#if 0
		// C++20
		#include <source_location>

		template<typename ...Params>
		inline void assert(bool assertion, Params ...messageParams, const std::source_location location = std::source_location::current()) {
			log::print_error("CRITICAL ERROR in ", location.file_name, ':', location.line, " in ", location.function_name, "() ", ...messageParams);
			while(true);
		}

	#else

		#define assert(ASSERTION, ...)  _assert(ASSERTION, __FILE__, __LINE__, __VA_ARGS__)

		template<typename ...Params>
		inline void _assert(bool assertion, const char *filename, unsigned linenumber, Params ...messageParams) {
			if(!assertion){
				log::print_error("CRITICAL ERROR in ", filename, ':', linenumber, ' ', messageParams...);
				while(true);
			}
		}
	#endif
#endif
