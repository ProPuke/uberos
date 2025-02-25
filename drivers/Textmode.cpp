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
		"      лл                        пппппппппппппппппп                      олн     ",
		"       лл                                                              лл       ",
		"        олл                                                          ллн        ",
		"          олл                                                      ллн          ",
		"            олл                                                  ллн            ",
		"              олл                                              ллн              ",
		"                олл                                          ллн                ",
		"                  олллн                                  олллн                  ",
		"                      лллл                            лллл                      ",
		"                          ллллллл              ллллллл                          ",
		"                                лллллллллллллллл                                "
	};
}

namespace driver {
	namespace {
		auto abs(I32 x) -> I32 { return x<0?-x:x; }

		void _set_char(Textmode &textmode, U32 row, U32 col, U32 foreground, U32 background, U8 c) {
			auto oldChar = textmode.get_char(row, col);

			if(background==textmode.consoleOut.defaultBackgroundColour){
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
			}

			textmode.set_char(row, col, foreground, background, c);
		}
	}

	void Textmode::write_text(Cursor &cursor, U32 foreground, U32 background, WrapMode wrapMode, const char *text, bool autoScroll) {
		const auto rows = get_rows();

		if(cursor.row>=rows||cursor.col>=cursor.colEnd) return;

		switch(wrapMode){
			case WrapMode::none: {
				auto fromChar = text;
				auto fromCol = cursor.col;

				auto c = text;
				for(;*c&&cursor.col<cursor.colEnd;c++,cursor.col++) {
					if(*c=='\n'){
						if(c-1>fromChar){
							set_chars(cursor.row, fromCol, foreground, background, c-1-fromChar, (const U8*)fromChar);
						}
						cursor.row++;
						cursor.col = cursor.colStart;
						fromChar = c+1;
						fromCol = cursor.col;

						if(cursor.row>=rows) {
							if(autoScroll){
								Textmode::scroll(-1, 0, foreground, background);
								cursor.row = rows-1;
								cursor.col = cursor.colStart;
								fromCol = cursor.col;
							}else{
								break;
							}
						}

					}else{
						cursor.col++;
					}
				}

				set_chars(cursor.row, fromCol, foreground, background, c-fromChar, (const U8*)fromChar);
			} break;
			case WrapMode::endOfLine: {
				auto fromChar = text;
				auto fromCol = cursor.col;

				auto c = text;
				for(;*c&&cursor.row<rows;c++) {
					if(*c=='\n'){
						if(c-1>fromChar){
							set_chars(cursor.row, fromCol, foreground, background, c-1-fromChar, (const U8*)fromChar);
						}
						cursor.row++;
						cursor.col = cursor.colStart;
						fromChar = c+1;
						fromCol = cursor.col;

					}else{
						cursor.col++;
						if(cursor.col>=cursor.colEnd){
							set_chars(cursor.row, fromCol, foreground, background, c-fromChar, (const U8*)fromChar);
							cursor.row++;
							cursor.col = cursor.colStart;	
							fromChar = c;
							fromCol = cursor.col;
						}
					}

					if(cursor.row>=rows) {
						if(autoScroll){
							Textmode::scroll(-1, 0, foreground, background);
							cursor.row = rows-1;
							cursor.col = cursor.colStart;
							fromCol = cursor.col;
						}else{
							break;
						}
					}
				}

				set_chars(cursor.row, fromCol, foreground, background, c-fromChar, (const U8*)fromChar);
			} break;
			case WrapMode::whitespace: {
				auto lastChar = text;
				auto lastCharCol = cursor.col;

				auto fromChar = text;
				auto fromCol = cursor.col;

				auto c = text;
				for(;*c&&cursor.col<cursor.colEnd;c++) {
					if(*c=='\n'){
						set_chars(cursor.row, fromCol, foreground, background, c-fromChar, (const U8*)fromChar);

						lastChar++; //skip over this char

						cursor.row++;
						cursor.col = cursor.colStart;
						fromChar = c+1;
						fromCol = lastCharCol = cursor.col;
						if(cursor.row>=rows) {
							if(autoScroll){
								Textmode::scroll(-1, 0, foreground, background);
								cursor.row = rows-1;
								cursor.col = cursor.colStart;
								fromCol = cursor.col;
							}else{
								break;
							}
						}

					}else if(*c==' '||*c=='-'){
						lastCharCol += (c-lastChar);
						lastChar = c;

						cursor.col++;

						if(cursor.col>=cursor.colEnd){
							set_chars(cursor.row, fromCol, foreground, background, c-fromChar, (const U8*)fromChar);

							if(*c==' ') lastChar++; //skip over this char

							cursor.row++;
							cursor.col = cursor.colStart;
							lastCharCol = cursor.col;
							fromChar = c;
							fromCol = cursor.col;
							if(cursor.row>=rows) {
								if(autoScroll){
									Textmode::scroll(-1, 0, foreground, background);
									cursor.row = rows-1;
									cursor.col = cursor.colStart;
									fromCol = cursor.col;
								}else{
									break;
								}
							}

						}else{
							lastChar = c+1;
							lastCharCol = cursor.col;
						}

					}else{
						cursor.col++;

						if(cursor.col>=cursor.colEnd){
							//word started at beginning of line (and so longer than the line?)
							if(lastCharCol==cursor.colStart){
								//write out a partial, to do a wordbreak
								set_chars(cursor.row, fromCol, foreground, background, c-fromChar, (const U8*)fromChar);

								fromChar = c;
								fromCol = cursor.col;

							} else {
								set_chars(cursor.row, fromCol, foreground, background, lastChar-fromChar, (const U8*)fromChar);

								fromChar = lastChar;
								fromCol = lastCharCol;

								//fill the remainder of the line with spaces (so the word can start on the next line)
								U8 spaces[cursor.col-lastCharCol];
								for(auto i=0u;i<sizeof(spaces);i++){
									spaces[i] = ' ';
								}
								set_chars(cursor.row, fromCol, foreground, background, sizeof(spaces), spaces);
							}

							cursor.row++;
							cursor.col = cursor.colStart;
							lastCharCol = cursor.col;
							fromCol = cursor.col;

							if(cursor.row>=rows) {
								if(autoScroll){
									Textmode::scroll(-1, 0, foreground, background);
									cursor.row = rows-1;
									cursor.col = cursor.colStart;
									fromCol = cursor.col;
								}else{
									break;
								}
							}
						}
					}
				}

				set_chars(cursor.row, fromCol, foreground, background, c-fromChar, (const U8*)fromChar);
			} break;
		}
	}

	void Textmode::clear() {
		clear(consoleOut.foregroundColour, consoleOut.backgroundColour, ' ');
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

	auto Textmode::_on_start() -> Try<> {
		consoleOutCursor = Cursor{0,0,0,get_cols()};

		return Super::_on_start();
	}
}
