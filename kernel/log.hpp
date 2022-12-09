#pragma once

#ifndef STDIO_COLOUR
	#define STDIO_COLOUR 1
#endif

namespace log {
	enum struct PrintType {
		info,
		debug,
		warning,
		error
	};

	/*  */ void print_start(PrintType type);
	inline void print_info_start() { return print_start(PrintType::info); }
	inline void print_debug_start() { return print_start(PrintType::debug); }
	inline void print_warning_start() { return print_start(PrintType::warning); }
	inline void print_error_start() { return print_start(PrintType::error); }
	/*  */ void print_end();

	template<typename ...Params>
	void print_inline(Params ...params);

	template<typename ...Params>
	void print(PrintType type, Params ...params){
		print_start(type);
		print_inline(params...);
		print_end();
	}

	template<typename ...Params> void print_info(Params ...params){ return print(PrintType::info, params...); }
	template<typename ...Params> void print_debug(Params ...params){ return print(PrintType::debug, params...); }
	template<typename ...Params> void print_warning(Params ...params){ return print(PrintType::warning, params...); }
	template<typename ...Params> void print_error(Params ...params){ return print(PrintType::error, params...); }

	template<typename ...Params> void beginSection(Params ...params);
	/*                        */ void endSection();

	struct Section {
		template<typename ...Params>
		/**/ Section(Params ...params){
			beginSection(params...);
		}

		/**/~Section(){
			endSection();
		}
	};
}

#include "log.inl"
