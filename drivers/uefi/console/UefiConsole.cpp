#include "UefiConsole.hpp"

#include <kernel/arch/uefi/uefi.hpp>

namespace driver::console {
	namespace {
		UMax rows = 0;
		UMax cols = 0;

		const auto colourCount = 16u;

		const U32 colours[colourCount] = {
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

	auto UefiConsole::get_mode_count() -> U32 {
		return uefi::systemTable->conOut->mode->maxMode;
	}

	auto UefiConsole::get_mode(U32 mode) -> Mode {
		UMax rows;
		UMax cols;
		uefi::systemTable->conOut->queryMode(mode, &cols, &rows);

		return Mode{(U32)rows, (U32)cols, colourCount};
	}

	void UefiConsole::set_mode(U32 mode) {
		uefi::systemTable->conOut->setMode(mode);
	}

	auto UefiConsole::get_rows() -> U32 {
		return rows;
	}

	auto UefiConsole::get_cols() -> U32 {
		return cols;
	}

	auto UefiConsole::get_colour_count() -> U32 {
		return colourCount;
	}
	auto get_colour(U32 x) -> U32 {
		return colours[x];
	}

	void UefiConsole::write_text(U32 foreground, U32 background, const char *s) {
		uefi::systemTable->conOut->setAttribute((uefi::TextAttribute)(((U32)uefi::TextAttribute::blackForeground+foreground)|((U32)uefi::TextAttribute::blackBackground+background%8)));
		static C16 buffer[64];
		auto length = strlen(s);
		while(length>0){
			auto i=0u;
			for(;i<sizeof(buffer)/sizeof(buffer[0])&&length>0;i++,length--){
				buffer[i]=s[i];
			}
			buffer[i] = '\0';
			uefi::systemTable->conOut->outputString(buffer);
		}
	}

	void UefiConsole::clear(U32 foreground, U32 background) {
		uefi::systemTable->conOut->setAttribute((uefi::TextAttribute)(((U32)uefi::TextAttribute::blackForeground+foreground)|((U32)uefi::TextAttribute::blackBackground+background%8)));
		uefi::systemTable->conOut->clearScreen();
	}

	auto UefiConsole::_on_start() -> Try<> {
		auto mode = uefi::systemTable->conOut->mode->mode;
		uefi::systemTable->conOut->queryMode(mode, &cols, &rows);

		return Super::_on_start();
	}
}
