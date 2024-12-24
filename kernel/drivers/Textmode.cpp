#include "Textmode.hpp"

#include <kernel/console.hpp>

namespace {
	const char *art[] = {
		// "                                ****************                                ",
		// "                          *******              *******                          ",
		// "                      ****                            ****                      ",
		// "                   ****                                  ****                   ",
		// "                 **                                          **                 ",
		// "               **                                              **               ",
		// "             **                                                  **             ",
		// "           **                                                      **           ",
		// "         **                                                          **         ",
		// "       **                                                              **       ",
		// "      **                                                                **      ",
		// "     **                            .::.   .::.                           **     ",
		// "    **                             ::::   ::::                            **    ",
		// "   **                 ':::::::::'  '::'   '::'  ':::::::::'                **   ",
		// "   **                  :::::::::                 :::::::::                 **   ",
		// "  **                   :::::::::                 :::::::::                  **  ",
		// "  **                   :::::::::                 :::::::::                  **  ",
		// " **                    :::::::::                 :::::::::                   ** ",
		// " **                    :::::::::                 :::::::::                   ** ",
		// " **                    :::::::::                 :::::::::                   ** ",
		// " **                    :::::::::                 :::::::::                   ** ",
		// " **                    :::::::::                 :::::::::                   ** ",
		// " **                    :::::::::                 :::::::::                   ** ",
		// " **                    '::::::::                 ::::::::'                   ** ",
		// "  **                    ::::::::.               .::::::::                   **  ",
		// "  **                    '::::::::               ::::::::'                   **  ",
		// "   **                    :::::::::..         ..:::::::::                   **   ",
		// "   **                    ':::::::::::::::::::::::::::::'                   **   ",
		// "    **                    ':::::::::::::::::::::::::::'                   **    ",
		// "     **                     ':::::::::::::::::::::::'                    **     ",
		// "      **                       ':::::::::::::::::'                      **      ",
		// "       **                                                              **       ",
		// "         **                                                          **         ",
		// "           **                                                      **           ",
		// "             **                                                  **             ",
		// "               **                                              **               ",
		// "                 **                                          **                 ",
		// "                   ****                                  ****                   ",
		// "                      ****                            ****                      ",
		// "                          *******              *******                          ",
		// "                                ****************                                "
		"                                лллллллллллллллл                                ",
		"                          ллллллл              ллллллл                          ",
		"                      лллл                            лллл                      ",
		"                   лллл                                  лллл                   ",
		"                олл                                          ллн                ",
		"              олл                                              ллн              ",
		"            олл                                                  ллн            ",
		"          олл                                                      ллн          ",
		"        олл                                                          ллн        ",
		"       лл                                                              лл       ",
		"      лл                                                                лл      ",
		"     лл                             мм     мм                            лл     ",
		"    лл                             лллл   лллл                            лл    ",
		"   лл                 олллллллллн   пп     пп   олллллллллн                лл   ",
		"  олн                  ллллллллл                 ллллллллл                 олн  ",
		"  лл                   ллллллллл                 ллллллллл                  лл  ",
		"  лл                   ллллллллл                 ллллллллл                  олн ",
		" олн                   ллллллллл                 ллллллллл                   лл ",
		" лл                    ллллллллл                 ллллллллл                   лл ",
		" лл                    ллллллллл                 ллллллллл                   лл ",
		" лл                    ллллллллл                 ллллллллл                   лл ",
		" лл                    ллллллллл                 ллллллллл                   лл ",
		" лл                    ллллллллл                 ллллллллл                   лл ",
		" лл                    ллллллллл                 ллллллллл                   лл ",
		" олн                   оллллллллн               оллллллллн                  олн ",
		"  лл                    ллллллллл               ллллллллл                   лл  ",
		"  олн                   олллллллллмм         ммлллллллллн                  олн  ",
		"   лл                    олллллллллллллллллллллллллллллн                   лл   ",
		"    лл                    олллллллллллллллллллллллллллн                   лл    ",
		"     лл                     олллллллллллллллллллллллн                    олн    ",
		"      лл                        пппппппппппппппппп                      олн      ",
		"       лл                                                              лл       ",
		"        олл                                                          ллн        ",
		"          олл                                                      ллн          ",
		"            олл                                                  ллн            ",
		"              олл                                              ллн              ",
		"                олл                                          ллн                ",
		"                  олллн                                  олллн                  ",
		"                      лллл                            лллл                      ",
		"                          ллллллл              олллллл                          ",
		"                                лллллллллллллллл                                "
	};
}

namespace driver {
	namespace {
		auto abs(I32 x) -> I32 { return x<0?-x:x; }

		void _set_char(Textmode &textmode, U32 row, U32 col, U32 foreground, U32 background, U8 c) {
			auto oldChar = textmode.get_char(row, col);

			if(oldChar==(U8)'л'){
				if(c==' '){
					if(background==textmode.get_char_background(row, col)&&foreground!=textmode.get_char_foreground(row, col)){
						return;
					}

				}else{
					auto oldForeground = textmode.get_char_foreground(row, col);
					if(foreground!=oldForeground){
						textmode.set_char(row, col, foreground, oldForeground, c);
						return;
					}
				}

			}else if(c==' '&&(oldChar=='о'||oldChar=='н'||oldChar=='м'||oldChar=='п')){
				if(background==textmode.get_char_background(row, col)&&foreground!=textmode.get_char_foreground(row, col)){
					return;
				}
			}

			textmode.set_char(row, col, foreground, background, c);
		}
	}

	DriverType Textmode::driverType{"textmode", &Super::driverType};

	/**/ Textmode::Textmode(const char *name, const char *description):
		Driver(name, description)
	{
		type = &driverType;
	}

	auto Textmode::get_nearest_colour(U32 colour) -> U32 {
		U32 nearestColour = 0;
		U32 nearestDistance = ~0;

		const int r = (colour >> 16) & 0xff;
		const int g = (colour >>  8) & 0xff;
		const int b = (colour >>  0) & 0xff;

		const I32 colour_count = get_colour_count();
		for(auto i=0;i<colour_count;i++){
			const auto compare = get_colour(i);
			const int compareR = (compare >> 16) & 0xff;
			const int compareG = (compare >>  8) & 0xff;
			const int compareB = (compare >>  0) & 0xff;

			const U32 distance = abs(r-compareR)+abs(g-compareG)+abs(b-compareB);
			if(distance<nearestDistance){
				nearestColour = i;
				nearestDistance = distance;
			}
		}

		return nearestColour;
	}

	void Textmode::write_text(Cursor &cursor, U32 foreground, U32 background, WrapMode wrapMode, const char *text, bool autoScroll) {
		const auto rows = get_rows();

		if(cursor.row>=rows||cursor.col>=cursor.colEnd) return;

		switch(wrapMode){
			case WrapMode::none:
				for(auto c=text;*c&&cursor.col<cursor.colEnd;c++,cursor.col++) {
					if(*c=='\n'){
						cursor.row++;
						cursor.col = cursor.colStart;

						if(cursor.row>=rows) {
							if(autoScroll){
								Textmode::scroll(-1, 0, foreground, background);
								cursor.row = rows-1;
								cursor.col = cursor.colStart;
							}else{
								break;
							}
						}
					}else{
						_set_char(*this, cursor.row, cursor.col, foreground, background, *c);
						cursor.col++;
					}
				}
			break;
			case WrapMode::endOfLine:
				for(auto c=text;*c&&cursor.row<rows;c++) {
					if(*c=='\n'){
						cursor.row++;
						cursor.col = cursor.colStart;

					}else{
						_set_char(*this, cursor.row, cursor.col, foreground, background, *c);
						cursor.col++;
						if(cursor.col>=cursor.colEnd){
							cursor.row++;
							cursor.col = cursor.colStart;
						}
					}

					if(cursor.row>=rows) {
						if(autoScroll){
							Textmode::scroll(-1, 0, foreground, background);
							cursor.row = rows-1;
							cursor.col = cursor.colStart;
						}else{
							break;
						}
					}
				}
			break;
			case WrapMode::whitespace: {
				auto lastChar = text;
				auto lastCharCol = cursor.col;
				for(auto c=text;*c&&cursor.col<cursor.colEnd;c++,cursor.col++) {
					if(*c=='\n'){
						for(;lastChar<c;lastChar++,lastCharCol++){
							_set_char(*this, cursor.row, lastCharCol, foreground, background, *lastChar);
						}

						lastChar++; //skip over this char

						cursor.row++;
						cursor.col = cursor.colStart;
						lastCharCol = cursor.col;
						if(cursor.row>=rows) {
							if(autoScroll){
								Textmode::scroll(-1, 0, foreground, background);
								cursor.row = rows-1;
								cursor.col = cursor.colStart;
							}else{
								break;
							}
						}

					}else if(*c==' '||*c=='-'){
						for(;lastChar<c;lastChar++,lastCharCol++){
							_set_char(*this, cursor.row, lastCharCol, foreground, background, *lastChar);
						}

						if(cursor.col>=cursor.colEnd){
							if(*c==' ') lastChar++; //skip over this char

							cursor.row++;
							cursor.col = cursor.colStart;
							lastCharCol = cursor.col;
							if(cursor.row>=rows) {
								if(autoScroll){
									Textmode::scroll(-1, 0, foreground, background);
									cursor.row = rows-1;
									cursor.col = cursor.colStart;
								}else{
									break;
								}
							}

						}else{
							_set_char(*this, cursor.row, cursor.col, foreground, background, *c);

							cursor.col++;
							lastChar = c+1;
							lastCharCol = cursor.col;
						}

					}else{
						cursor.col++;

						if(cursor.col>=cursor.colEnd){
							//word started at beginning of line (and so longer than the line?)
							if(lastCharCol==cursor.colStart){
								//write out a partial, to do a wordbreak
								for(;lastChar<=c;lastChar++,lastCharCol++){
									_set_char(*this, cursor.row, lastCharCol, foreground, background, *lastChar);
								}
							} else {
								//fill the remainder of the line with spaces (so the word can start on the next line)
								for(auto spaceCol = lastCharCol;spaceCol<cursor.col;spaceCol++){
									_set_char(*this, cursor.row, spaceCol, foreground, background, ' ');
								}
							}

							cursor.row++;
							cursor.col = cursor.colStart;
							lastCharCol = cursor.col;
							if(cursor.row>=rows) {
								if(autoScroll){
									Textmode::scroll(-1, 0, foreground, background);
									cursor.row = rows-1;
									cursor.col = cursor.colStart;
								}else{
									break;
								}
							}
						}

						_set_char(*this, cursor.row, cursor.col, foreground, background, *c);
						cursor.col++;
					}
				}
			} break;
		}
	}

	void Textmode::clear(U32 foreground, U32 background, char bgChar) {
		fill(0, 0, get_rows(), get_cols(), foreground, background, bgChar);

		auto fg = get_nearest_colour(0x0000e0);
		auto bg = get_nearest_colour(0x000060);

		const auto rowLines = sizeof(art)/sizeof(art[0]);

		auto skip = rowLines>25?(rowLines-25)/2:0;
		for(auto row=skip;row<rowLines-skip;row++){
			for(auto col=0u;col<80;col++){
				set_char(row-skip, col, fg, bg, art[row][col]);
			}
		}
	}

	void Textmode::fill(U32 row, U32 colStart, U32 rowCount, U32 colCount, U32 foreground, U32 background, U8 bgChar) {
		for(;rowCount>0;row++,rowCount--){
			for(auto col=colStart;col<colStart+colCount;col++){
				set_char(row, col, foreground, background, bgChar);
			}
		}
	}

	void Textmode::scroll(I32 scrollRows, I32 scrollCols, U32 foreground, U32 background, U8 bgChar) {
		scroll_region(0, 0, get_rows(), get_cols(), scrollRows, scrollCols, foreground, background, bgChar);
	}

	void Textmode::scroll_region(U32 _startRow, U32 _startCol, U32 rows, U32 cols, I32 scrollRows, I32 scrollCols, U32 foreground, U32 background, U8 bgChar) {
		I32 startRow = _startRow;
		I32 startCol = _startCol;
		I32 endRow = startRow+rows;
		I32 endCol = startCol+cols;

		auto rowDir = scrollRows<=0?+1:-1;
		auto colDir = scrollCols<=0?+1:-1;

		for(I32 row=rowDir>=0?startRow:endRow-1;rowDir>=0?row<endRow:row>=startRow;row+=rowDir)
		for(I32 col=colDir>=0?startCol:endCol-1;colDir>=0?col<endCol:col>=startCol;col+=colDir) {
			auto oldRow = row-scrollRows;
			auto oldCol = col-scrollCols;

			if(oldCol<startCol||oldRow<startRow||oldCol>=endCol||oldRow>=endRow){
				set_char(row, col, foreground, background, bgChar);
			}else{
				auto c = get_char(oldRow, oldCol);
				auto foreground = get_char_foreground(oldRow, oldCol);
				auto background = get_char_background(oldRow, oldCol);
				set_char(row, col, foreground, background, c);
			}
		}
	}

	void print_ansi(Textmode &textmode, const char *str) {
		auto &context = textmode.consoleContext;

		for(auto c=str;*c;c++){
			if(*c=='\x1b'||context.currentEscapeLength>0) {
				context.currentEscape[context.currentEscapeLength++] = *c;

				// if invalid, dump sequence to screen and reset
				if(
					context.currentEscapeLength==2&&context.currentEscape[1]!='['|| // not a sequence?
					context.currentEscapeLength+1==sizeof(context.currentEscape)/sizeof(context.currentEscape[0]) // out of space?
				){
					context.currentEscape[context.currentEscapeLength] = '\0';
					textmode.write_text(context.cursor, context.foreground, context.background, Textmode::WrapMode::endOfLine, context.currentEscape, true);
					context.currentEscapeLength = 0;
					continue;
				}

				if(*c>='a'&&*c<='z'||*c>='A'&&*c<='Z'){
					switch(*c){
						case 'm': { // graphic rendition
							auto read = &context.currentEscape[2];

							#define IS_END(x) (read[x]==';'||read[x]=='m')
							#define IS_1_DIGIT IS_END(1)
							#define IS_2_DIGIT (read[1]&&!IS_END(1)&&IS_END(2))
							#define IS_3_DIGIT (read[1]&&!IS_END(1)&&read[2]&&!IS_END(2)&&IS_END(3))

							while(true){
								switch(*read){
									case '0':
										if(IS_1_DIGIT){
											context.foreground = context.defaultForeground;
											context.background = context.defaultBackground;
											context.ansiBold = false;
											//TODO: reevaluate colours as non-bold/bright
										}
									break;
									case '1':
										if(IS_1_DIGIT){
											context.ansiBold = true;
											//TODO: reevaluate colours as bold/bright

										}else if(IS_3_DIGIT){
											read+=2;
											switch(*read){
												case '0' ... '7':
													context.foreground = textmode.brightAnsiColours[*read-'0'];
												break;
											}
										}
									break;
									case '2':
										if(IS_2_DIGIT){
											read++;
											switch(*read){
												case '2':
													context.ansiBold = false;
												break;
											}
										}
									break;
									case '3':
										if(IS_2_DIGIT){
											read++;
											switch(*read){
												case '0' ... '7':
													context.foreground = (context.ansiBold&&false?textmode.brightAnsiColours:textmode.ansiColours)[*read-'0'];
												break;
												case '8':
													//TODO: RGB colour
												break;
												case '9':
													context.foreground = context.defaultForeground;
												break;
											}
										}
									break;
									case '4':
										if(IS_2_DIGIT){
											read++;
											switch(*read){
												case '0' ... '7':
													context.background = (context.ansiBold&&false?textmode.brightAnsiColours:textmode.ansiColours)[*read-'0'];
												break;
												case '8':
													//TODO: RGB colour
												break;
												case '9':
													context.background = context.defaultBackground;
												break;
											}
										}
									break;
									case '9':
										if(IS_2_DIGIT){
											read++;
											switch(*read){
												case '0' ... '7':
													context.foreground = textmode.brightAnsiColours[*read-'0'];
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

					context.currentEscapeLength = 0;
					continue;
				}

			}else{
				char string[2] = {*c, '\0'};
				textmode.write_text(context.cursor, context.foreground, context.background, Textmode::WrapMode::endOfLine, string, true);
			}
		}
		// textmode.write_text(cursor, foreground, background, Textmode::WrapMode::endOfLine, str, true);
	}

	void Textmode::bind_to_console() {
		if(!api.is_active()) return;

		consoleContext.currentEscapeLength = 0;

		console::bind(this,
			[](void *_textmode, char c) {
				auto &textmode = *(driver::Textmode*)_textmode;
				char string[2] = {c, '\0'};
				// debug::halt();
				// textmode.write_text(textmode.consoleContext.cursor, textmode.consoleContext.foreground, textmode.consoleContext.background, Textmode::WrapMode::endOfLine, string, true);
				print_ansi(textmode, string);
			},
			nullptr,
			nullptr,
			[](void *_textmode, const char *str) {
				auto &textmode = *(driver::Textmode*)_textmode;
				// debug::halt();
				// textmode.write_text(textmode.consoleContext.cursor, textmode.consoleContext.foreground, textmode.consoleContext.background, Textmode::WrapMode::endOfLine, str, true);
				print_ansi(textmode, str);
			},
			nullptr
		);
	}

	auto Textmode::_on_start() -> bool {
		consoleContext.cursor = Cursor{0,0,0,get_cols()};
		consoleContext.defaultForeground = get_nearest_colour(0xa0a0a0);
		// consoleContext.defaultBackground = get_nearest_colour(0x000000);
		consoleContext.defaultForeground = get_nearest_colour(0x0000ff);
		consoleContext.defaultBackground = get_nearest_colour(0x000080);
		consoleContext.defaultForeground = get_nearest_colour(0xa0a0a0);
		consoleContext.foreground = consoleContext.defaultForeground;
		consoleContext.background = consoleContext.defaultBackground;

		ansiColours[0] = get_nearest_colour(0x000000);
		ansiColours[1] = get_nearest_colour(0x990000);
		ansiColours[2] = get_nearest_colour(0x00a600);
		ansiColours[3] = get_nearest_colour(0x999900);
		ansiColours[4] = get_nearest_colour(0x0000b2);
		ansiColours[5] = get_nearest_colour(0xb200b2);
		ansiColours[6] = get_nearest_colour(0x00a6b2);
		ansiColours[7] = get_nearest_colour(0xa0a0a0);

		brightAnsiColours[0] = get_nearest_colour(0x666666);
		brightAnsiColours[1] = get_nearest_colour(0xff0000);
		brightAnsiColours[2] = get_nearest_colour(0x00ff00);
		brightAnsiColours[3] = get_nearest_colour(0xffff00);
		brightAnsiColours[4] = get_nearest_colour(0x0000ff);
		brightAnsiColours[5] = get_nearest_colour(0xff00ff);
		brightAnsiColours[6] = get_nearest_colour(0x00ffff);
		brightAnsiColours[7] = get_nearest_colour(0xffffff);

		return true;
	}
}
