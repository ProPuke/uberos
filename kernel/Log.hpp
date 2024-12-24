#pragma once

#include <kernel/logging.hpp>

struct Log {
	const char *name;
	
	/**/ Log(const char *name):
		name(name)
	{}

	void print_inline(char x);
	void print_inline(char *x);
	void print_inline(const char *x);
	template<typename ...Params>
	void print_inline(Params ...params);
	void print_end();

	void print_info_start();
	void print_debug_start();
	void print_warning_start();
	void print_error_start();

	template<typename ...Params> void print_info(Params ...params);
	template<typename ...Params> void print_debug(Params ...params);
	template<typename ...Params> void print_warning(Params ...params);
	template<typename ...Params> void print_error(Params ...params);

	struct Section {
		template<typename ...Params>
		/**/ Section(Params ...params);
		/**/~Section();
	};

	template<typename ...Params>
	auto section(Params ...params) -> Section;
};

#include "Log.inl"
