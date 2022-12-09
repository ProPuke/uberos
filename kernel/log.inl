#include "log.hpp"

#include <common/format.hpp>
#include <common/types.hpp>
#include <common/stdlib.hpp>

#include <kernel/console.hpp>

namespace log {
	extern U32 indent;

	template<typename Type>
	inline void _print(Type x){
		console::puts(to_string(x));
	}
	template<> inline void _print(char x){
		console::putc(x);
	}
	template<> inline void _print(char *x){
		console::puts(x);
	}
	template<> inline void _print(const char *x){
		console::puts(x);
	}

	template<typename ...Params>
	void print_inline(Params ...params){
		(_print(params), ...);
	}

	inline void print_start(PrintType type){
		for(U32 i=0;i<indent;i++){
			_print(' ');
			_print(' ');
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

	inline void print_end() {
		#if STDIO_COLOUR == 1
			_print("\x1b[0m\n");
		#else
			_print('\n');
		#endif
	}

	template<typename ...Params>
	inline void beginSection(Params ...params){
		print_info(params...);
		indent++;
	}

	inline void endSection(){
		indent--;
		print_info("OK\n");
	}
}
