#pragma once

#include "panic.hpp"

namespace panic {
	template<typename Type>
	inline auto Panic::_print_details_inline(Type x) -> Panic& {
		_print_details_inline(to_string(x));
		return *this;
	}

	inline auto Panic::_print_details_inline(char x) -> Panic& {
		char str[2] = {x, '\0'};
		_print_details_inline(str);
		return *this;
	}

	inline auto Panic::_print_details_inline(char *x) -> Panic& {
		_print_details_inline((const char*)x);
		return *this;
	}

	template<typename ...Params>
	auto Panic::print_details(Params ...params) -> Panic& {
		print_details_start();
		print_details_inline(params...);
		print_details_end();

		return *this;
	}
	
	template<typename ...Params>
	auto Panic::print_details_inline(Params ...params) -> Panic& {
		(_print_details_inline(params), ...);

		return *this;
	}
}
