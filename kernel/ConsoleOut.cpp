#include "ConsoleOut.hpp"

void ConsoleOut::write_text(const char *str) {
	for(auto c=str;*c;c++){
		if(*c=='\x1b'||_currentEscapeLength>0) {
			_currentEscape[_currentEscapeLength++] = *c;

			// if invalid, dump sequence to screen and reset
			if(
				_currentEscapeLength==2&&_currentEscape[1]!='['|| // not a sequence?
				_currentEscapeLength+1==sizeof(_currentEscape)/sizeof(_currentEscape[0]) // out of space?
			){
				_currentEscape[_currentEscapeLength] = '\0';
				write_raw_text(_currentEscape);
				_currentEscapeLength = 0;
				continue;
			}

			if(*c>='a'&&*c<='z'||*c>='A'&&*c<='Z'){
				switch(*c){
					case 'm': { // graphic rendition
						auto read = &_currentEscape[2];

						#define IS_END(x) (read[x]==';'||read[x]=='m')
						#define IS_1_DIGIT IS_END(1)
						#define IS_2_DIGIT (read[1]&&!IS_END(1)&&IS_END(2))
						#define IS_3_DIGIT (read[1]&&!IS_END(1)&&read[2]&&!IS_END(2)&&IS_END(3))

						while(true){
							switch(*read){
								case '0':
									if(IS_1_DIGIT){
										clear_foreground_colour();
										clear_background_colour();
										set_bold(false);
										//TODO: reevaluate colours as non-bold/bright
									}
								break;
								case '1':
									if(IS_1_DIGIT){
										set_bold(true);
										//TODO: reevaluate colours as bold/bright

									}else if(IS_3_DIGIT){
										read+=2;
										switch(*read){
											case '0' ... '7':
												set_foreground_colour(brightAnsiColours[*read-'0']);
											break;
										}
									}
								break;
								case '2':
									if(IS_2_DIGIT){
										read++;
										switch(*read){
											case '2':
												set_bold(false);
											break;
										}
									}
								break;
								case '3':
									if(IS_2_DIGIT){
										read++;
										switch(*read){
											case '0' ... '7':
												set_foreground_colour((boldText&&false?brightAnsiColours:ansiColours)[*read-'0']);
											break;
											case '8':
												//TODO: RGB colour
											break;
											case '9':
												clear_foreground_colour();
											break;
										}
									}
								break;
								case '4':
									if(IS_2_DIGIT){
										read++;
										switch(*read){
											case '0' ... '7':
												set_background_colour((boldText&&false?brightAnsiColours:ansiColours)[*read-'0']);
											break;
											case '8':
												//TODO: RGB colour
											break;
											case '9':
												clear_background_colour();
											break;
										}
									}
								break;
								case '9':
									if(IS_2_DIGIT){
										read++;
										switch(*read){
											case '0' ... '7':
												set_foreground_colour(brightAnsiColours[*read-'0']);
											break;
										}
									}
								break;
								default:
								break;
							}

							read++;

							if(*read!=';') break;
						};

					} break;
					default:
						//NOT SUPPORTED. SKIP IT
					break;
				}

				_currentEscapeLength = 0;
				continue;
			}

		}else{
			char string[2] = {*c, '\0'};
			write_raw_text(string);
		}
	}
	// write_raw_text(str);
}
