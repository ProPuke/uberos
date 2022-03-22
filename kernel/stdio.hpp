#pragma once

#include <common/stdlib.hpp>
#include <common/format.hpp>

#undef getc
#undef putc
#undef gets
#undef puts

#ifndef STDIO_COLOUR
	#define STDIO_COLOUR 1
#endif

namespace stdio {
	extern U32 indent;

	typedef auto (*Putc)(unsigned char c) -> void;
	typedef auto (*Getc)() -> unsigned char;
	typedef auto (*Puts)(const char *str) -> void;
	typedef void (*Gets)(char *buf, U32 length);

	extern Putc putc;
	extern Getc getc;
	extern Puts puts;
	extern Gets gets;

	void bind(Putc putc, Getc getc, Puts puts = nullptr, Gets gets = nullptr);
	// void bind(typeof putc, typeof getc, typeof puts = nullptr, typeof gets = nullptr);

	enum struct PrintType {
		info,
		debug,
		warning,
		error
	};

	template<typename Type>
	inline void _print(Type x){ return puts(to_string(x)); }
	inline void _print(char x){ return putc(x); }
	inline void _print(const char *x){ return puts(x); }

	template<typename ...Params>
	void print_inline(Params ...params){
		(_print(params), ...);
	}

	inline void print_start(PrintType type){
		for(U32 i=0;i<indent;i++){
			putc(' ');
			putc(' ');
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
