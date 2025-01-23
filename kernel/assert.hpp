#pragma once

#ifndef DEBUG
	template<typename ...Params>
	inline void assert(bool assertion, Params ...messageParams) {}

#else
	#include <kernel/logging.hpp>

	#if 1
		// C++20
		#include <source_location>

		template<typename ...Params>
		inline void assert(bool assertion, Params ...messageParams, const std::source_location location = std::source_location::current()) {
			if(assertion) return;

			logging::print_error("CRITICAL ERROR in ", location.file_name(), ':', location.line(), " in ", location.function_name(), "() ", messageParams...);
			while(true);
		}

	#else

		#define assert(ASSERTION, ...)  _assert(ASSERTION, __FILE__, __LINE__, ##__VA_ARGS__)

		template<typename ...Params>
		inline void _assert(bool assertion, const char *filename, unsigned linenumber, Params ...messageParams) {
			if(assertion) return;

			logging::print_error("CRITICAL ERROR in ", filename, ':', linenumber, ' ', messageParams...);
			while(true);
		}
	#endif
#endif
