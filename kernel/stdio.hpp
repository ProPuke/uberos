#pragma once

#include <common/stdlib.hpp>
#include <common/format.hpp>

#undef getc
#undef putc
#undef gets
#undef _puts

#ifndef STDIO_COLOUR
	#define STDIO_COLOUR 1
#endif

namespace stdio {
	extern U32 indent;

	char getc();
	void _putc(char c);
	void _puts(const char *str);

	enum struct PrintType {
		info,
		debug,
		warning,
		error
	};

	template<typename Type>
	inline void _print(Type x){ return _puts(to_string(x)); }
	inline void _print(char x){ return _putc(x); }
	inline void _print(const char *x){ return _puts(x); }

	template<typename ...Params>
	void print_inline(Params ...params){
		(_print(params), ...);
	}

	inline void print_start(PrintType type){
		for(U32 i=0;i<indent;i++){
			_putc(' ');
			_putc(' ');
		};
		#if STDIO_COLOUR == 1
			switch(type){
				case PrintType::info:
				break;
				case PrintType::debug:
					_print("\x1b[33;1m");
				break;
				case PrintType::warning:
					_print("\x1b[31;1m");
				break;
				case PrintType::error:
					_print("\x1b[30;41;1m");
				break;
			}
		#endif
	}

	inline void print_info_start() { return print_start(PrintType::info); }
	inline void print_debug_start() { return print_start(PrintType::debug); }
	inline void print_warning_start() { return print_start(PrintType::warning); }
	inline void print_error_start() { return print_start(PrintType::error); }

	inline void print_end() {
		#if STDIO_COLOUR == 1
			_print("\x1b[0m\n");
		#else
			_print('\n');
		#endif
	}

	template<typename ...Params>
	void print(PrintType type, Params ...params){
		print_start(type);
		print_inline(params...);
		print_end();
	}

	template<typename ...Params>
	void print_info(Params ...params){ return print(PrintType::info, params...); }
	template<typename ...Params>
	void print_debug(Params ...params){ return print(PrintType::debug, params...); }
	template<typename ...Params>
	void print_warning(Params ...params){ return print(PrintType::warning, params...); }
	template<typename ...Params>
	void print_error(Params ...params){ return print(PrintType::error, params...); }

	template<typename ...Params>
	void beginSection(Params ...params){
		print_info(params...);
		indent++;
	}

	inline void endSection(){
		indent--;
		print_info("OK\n");
	}

	void gets(char *buf, int buflen);

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
