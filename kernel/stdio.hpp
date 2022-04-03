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

	typedef auto (*Putc)(void*, unsigned char c) -> void;
	typedef auto (*Getc)(void*) -> unsigned char;
	typedef auto (*Peekc)(void*) -> unsigned char;
	typedef auto (*Puts)(void*, const char *str) -> void;
	typedef void (*Gets)(void*, char *buf, U32 length);

	extern void* _binding;
	extern Putc _binding_putc;
	extern Getc _binding_peekc;
	extern Getc _binding_getc;
	extern Puts _binding_puts;
	extern Gets _binding_gets;

	inline auto putc(unsigned char c) -> void {
		return _binding_putc(_binding, c);
	}
	inline auto peekc() -> unsigned char {
		return _binding_peekc(_binding);
	}
	inline auto getc() -> unsigned char {
		return _binding_getc(_binding);
	}
	inline auto puts(const char *str) -> void {
		return _binding_puts(_binding, str);
	}
	inline void gets(char *buffer, U32 length) {
		return _binding_gets(_binding, buffer, length);
	}

	void bind(void* binding, Putc putc, Peekc peekc, Getc getc, Puts puts = nullptr, Gets gets = nullptr);

	enum struct PrintType {
		info,
		debug,
		warning,
		error
	};

	template<typename Type>
	inline void _print(Type x){ return puts(to_string(x)); }
	inline void _print(char x){ return putc(x); }
	inline void _print(char *x){ return puts(x); }
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
