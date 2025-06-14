#pragma once

#ifndef DEBUG
	template<typename Type, typename ...Params>
	inline auto assert(Type test, Params ...messageParams) -> Type { return test; }

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

		template<typename Type, typename ...Params>
		inline auto _assert(Type test, const char *filename, unsigned linenumber, Params ...messageParams) -> Type {
			if(assertion) return test;

			logging::print_error("CRITICAL ERROR in ", filename, ':', linenumber, ' ', messageParams...);
			while(true);
		}
	#endif
#endif

template<typename Type, typename ...Params>
inline auto assert_return(Type value, Params ...messageParams) -> Type { assert(value, messageParams...); return value; }
