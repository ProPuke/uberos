#pragma once

#ifndef DEBUG
	template<typename ...Params>
	inline void assert(bool assertion, Params ...messageParams) {}

#else
	#include <kernel/logging.hpp>

	#if 1
		// C++20
		#include <source_location>

		struct AssertAssertion {
			bool assertion;
			std::source_location location;

			/**/ AssertAssertion(bool assertion, std::source_location location = std::source_location::current()):
				assertion(assertion),
				location(location)
			{}
		};

		template<typename ...Params>
		inline void assert(AssertAssertion assertion, Params ...messageParams) {
			if(assertion.assertion) return;

			logging::print_error("CRITICAL ERROR in ", assertion.location.file_name(), ':', assertion.location.line(), " in ", assertion.location.function_name(), "() ", messageParams...);
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
