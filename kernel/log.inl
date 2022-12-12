#include "log.hpp"

#include <common/format.hpp>
#include <common/types.hpp>
#include <common/stdlib.hpp>

#include <kernel/console.hpp>

namespace log {
	extern U32 indent;
	extern LList<Handler> handlers;

	template<typename Type>
	inline void _print(Type x){
		print_inline(to_string(x));
	}
	template<> inline void _print(char x){
		print_inline(x);
	}
	template<> inline void _print(char *x){
		print_inline(x);
	}
	template<> inline void _print(const char *x){
		print_inline(x);
	}

	inline void print_inline(char x){
		for(auto handler=handlers.head;handler;handler=handler->next){
			handler->print_inline_char(x);
		}
		console::putc(x);
	}
	
	inline void print_inline(const char * x){
		for(auto handler=handlers.head;handler;handler=handler->next){
			handler->print_inline_string(x);
		}
		console::puts(x);
	}

	template<typename ...Params>
	void print_inline(Params ...params){
		(_print(params), ...);
	}

	inline void print_start(PrintType type){
		for(auto handler=handlers.head;handler;handler=handler->next){
			handler->print_start(indent, type);
		}

		for(U32 i=0;i<indent;i++){
			console::putc(' ');
			console::putc(' ');
		}
		#if STDIO_COLOUR == 1
			switch(type){
				case PrintType::info:
				break;
				case PrintType::debug:
					console::puts("\x1b[33;1m");
				break;
				case PrintType::warning:
					console::puts("\x1b[31;1m");
				break;
				case PrintType::error:
					console::puts("\x1b[30;41;1m");
				break;
			}
		#endif
	}

	inline void print_end() {
		for(auto handler=handlers.head;handler;handler=handler->next){
			handler->print_end();
		}

		#if STDIO_COLOUR == 1
			console::puts("\x1b[0m\n");
		#else
			console::putc('\n');
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
