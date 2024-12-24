#include "console.hpp"

#include <common/stdlib.hpp>

namespace console {
	namespace {
		void null_putc(void*, char c) {}
		auto null_getc(void*) -> char { return 0; }
		auto null_peekc(void*) -> char { return 0; }

		void automatic_puts(void *binding, const char *str) {
			while(*str) _binding_putc(binding, *str++);
		}

		void automatic_gets(void *binding, char *buffer, U32 length) {
			char c;
			U32 i=0;

			while(true){
				if(i>=length){
					i = length-1;
				}

				c = _binding_getc(binding);

				if(c=='\r'||c=='\n') break;

				if(c=='\e'){
					c = _binding_getc(binding);
					if(c=='['){
						C8 params[7] = {};
						U8 paramCount = 0;

						c = _binding_getc(binding);

						//for FE sequences we'll read up to 7 params, then just assume the next character is a control code if not a valid param, or our 7 is used up..
						while(paramCount<sizeof(params)&&(c>=0x30&c<=0x3F||c>=0x20&c<=0x2F)){
							params[paramCount++] = c;
							c = _binding_getc(binding);
						}

						//all we care about for input (ANSI, vt and xterm sequences)
						switch(c){
							case 'A': //cursor up
								//param is numerical count if present (or 1)
							break;
							case 'B': //cursor down
								//param is numerical count if present (or 1)
							break;
							case 'C': //cursor right
								//param is numerical count if present (or 1)
							break;
							case 'D': //cursor left
								//param is numerical count if present (or 1)
							break;

							case '~': //vt sequence
								if(paramCount==1){
									switch(params[0]){
										case '1': //home
										break;
										case '2': //insert
										break;
										case '3': //delete
										break;
										case '4': //end
										break;
										case '5': //page up
										break;
										case '6': //page down
										break;
										case '7': //home
										break;
										case '8': //end
										break;
									}
								}
							break;
						}
					}

				}else if(c>=0x20&&c<=0x7e){
					_binding_putc(binding, c);
					buffer[i++] = c;

				}else if(c==0x09){
					//tab

				}else if(c==0x7f){
					//backspace
					if(i>0){
						i--;
						_binding_puts(binding, "\e[D \e[D");
						continue;
					}
			
				}else{
					//unknown
					// log::print_debug((U32)c);
				}
			}

			// _binding_putc(binding, '\n');
			buffer[i] = '\0';
		}
	}

	void *_binding = nullptr;
	PutcBinding _binding_putc = null_putc;
	PeekcBinding _binding_peekc = null_peekc;
	GetcBinding _binding_getc = null_getc;
	PutsBinding _binding_puts = automatic_puts;
	GetsBinding _binding_gets = automatic_gets;

	void bind(void* binding, PutcBinding putc, PeekcBinding peekc, GetcBinding getc, PutsBinding puts, GetsBinding gets) {
		_binding = binding;

		_binding_putc = putc?:null_putc;
		_binding_peekc = peekc?:null_peekc;
		_binding_getc = getc?:null_getc;

		_binding_puts = puts?:automatic_puts;
		_binding_gets = gets?:automatic_gets;
	}
}
