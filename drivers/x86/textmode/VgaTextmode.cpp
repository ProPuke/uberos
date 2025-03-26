#include "VgaTextmode.hpp"

namespace driver::textmode {
	namespace {
		const auto rows = 25; //TODO
		const auto cols = 80; //TODO

		const auto colour_count = 16;

		const U32 colours[colour_count] = {
			0x000000, // black
			0x000080, // blue
			0x008000, // green
			0x008080, // cyan
			0x800000, // red
			0x800080, // magenta
			0x808000, // brown
			0x808080, // light_grey
			0x404040, // dark_grey
			0x0000f0, // light_blue
			0x00f000, // light_green
			0x00f0f0, // light_cyan
			0xf00000, // light_red
			0xf000f0, // light_magenta
			0xf0f000, // light_brown
			0xffffff, // white
		};

		const auto physicalAddress = Physical<void>{0xb8000};

		VgaTextmode::Entry *buffer = nullptr;
	}

	auto VgaTextmode::get_mode_count() -> U32 {
		return 0;
	}

	auto VgaTextmode::get_mode(U32 mode) -> Mode {
		return {0, 0, 16};		
	}

	void VgaTextmode::set_mode(U32 mode) {
		;
	}

	auto VgaTextmode::_on_start() -> Try<> {
		if(auto superError = Super::_on_start(); !superError) return superError;

		textmode::buffer = TRY_RESULT(api.subscribe_memory<Entry>(physicalAddress, rows*cols*sizeof(Entry), mmu::Caching::writeCombining));

		return {};
	}

	auto VgaTextmode::_on_stop() -> Try<> {
		return {};
	}

	auto VgaTextmode::get_rows() -> U32 {
		return rows;
	}
	auto VgaTextmode::get_cols() -> U32 {
		return cols;
	}

	auto VgaTextmode::get_colour_count() -> U32 {
		return colour_count;
	}
	auto VgaTextmode::get_colour(U32 index) -> U32 {
		if(index >= colour_count) return 0x000000;
		return colours[index];
	}

	void VgaTextmode::set_char(U32 row, U32 col, U32 foreground, U32 background, U8 c) {
		get_entry(row, col) = {c, (U8)foreground, (U8)background};
	}

	void VgaTextmode::set_chars(U32 row, U32 col, U32 foreground, U32 background, U32 count, const U8 *chars) {
		Entry buffer[count];
		for(auto i=0u;i<count;i++){
			buffer[i].c = chars[i];
			buffer[i].bgColour = background;
			buffer[i].fgColour = foreground;
		}

		memcpy(&get_entry(row, col), buffer, sizeof(buffer));
	}

	auto VgaTextmode::get_char(U32 row, U32 col) -> U8 {
		return get_entry(row, col).c;
	}
	auto VgaTextmode::get_char_foreground(U32 row, U32 col) -> U32 {
		return get_entry(row, col).fgColour;
	}
	auto VgaTextmode::get_char_background(U32 row, U32 col) -> U32 {
		return get_entry(row, col).bgColour;
	}

	auto VgaTextmode::get_entry(U32 row, U32 col) -> Entry& {
		return buffer()[row*cols+col];
	}
	auto VgaTextmode::buffer() -> Entry* {
		return textmode::buffer;
	}

	void VgaTextmode::scroll_region(U32 _startRow, U32 _startCol, U32 rows, U32 cols, I32 scrollRows, I32 scrollCols, U32 foreground, U32 background, U8 bgChar) {
		I32 startRow = _startRow;
		I32 startCol = _startCol;
		I32 endRow = startRow+rows;

		auto rowDir = scrollRows<=0?+1:-1;

		Entry emptyRow[cols];
		for(auto i=0u;i<cols;i++){
			emptyRow[i].c = bgChar;
			emptyRow[i].bgColour = background;
			emptyRow[i].fgColour = foreground;
		}

		for(I32 row=rowDir>=0?startRow:endRow-1;rowDir>=0?row<endRow:row>=startRow;row+=rowDir) {
			auto oldRow = row-scrollRows;

			if(oldRow<startRow||oldRow>=endRow){
				memcpy(&get_entry(row, startCol), emptyRow, sizeof(emptyRow));
			}else{
				if(scrollCols>0){
					memcpy(&get_entry(row, startCol), emptyRow, sizeof(Entry)*scrollCols);
				}
				memcpy(&get_entry(row, startCol+(U32)max(0, scrollCols)), &get_entry(oldRow, startCol-scrollCols), sizeof(Entry)*(cols-maths::abs(scrollCols)));
				if(scrollCols<0){
					memcpy(&get_entry(row, cols+scrollCols), emptyRow, sizeof(Entry)*-scrollCols);
				}
			}
		}
	}
}
