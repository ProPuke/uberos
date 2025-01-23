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

		api.subscribe_memory((void*)_address, rows*cols*sizeof(Entry));

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
		return (Entry*)_address;
	}
}
