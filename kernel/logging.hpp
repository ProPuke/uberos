#pragma once

#include <common/LList.hpp>

#ifndef STDIO_COLOUR
	#define STDIO_COLOUR 1
#endif

namespace logging {
	enum struct PrintType {
		info,
		debug,
		warning,
		error
	};

	void print_start(PrintType type);
	void print_inline(char);
	void print_inline(char*);
	void print_inline(const char*);
	template<typename ...Params>
	void print_inline(Params ...params);
	void print_end();

	inline void print_info_start() { return print_start(PrintType::info); }
	inline void print_debug_start() { return print_start(PrintType::debug); }
	inline void print_warning_start() { return print_start(PrintType::warning); }
	inline void print_error_start() { return print_start(PrintType::error); }

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

	struct Handler: LListItem<Handler> {
		typedef void (*Print_start)(U32 indent, PrintType);
		typedef void (*Print_inline_char)(char);
		typedef void (*Print_inline_string)(const char*);
		typedef void (*Print_end)();

		/**/ Handler(Print_start print_start, Print_inline_char print_inline_char, Print_inline_string print_inline_string, Print_end print_end):
			print_start(print_start),
			print_inline_char(print_inline_char),
			print_inline_string(print_inline_string),
			print_end(print_end)
		{}

		Print_start print_start;
		Print_inline_char print_inline_char;
		Print_inline_string print_inline_string;
		Print_end print_end;
	};

	void init();

	void install_handler(Handler&);
	void uninstall_handler(Handler&);

	auto get_history_part_1() -> const char*;
	auto get_history_part_2() -> const char*;

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

#include "logging.inl"
