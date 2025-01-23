#pragma once

#include <kernel/drivers/Console.hpp>

#include <common/Try.hpp>

namespace driver {
	struct Textmode: driver::Console {
		DRIVER_TYPE(Textmode, "textmode", "Textmode Display Driver", driver::Console)

		enum struct WrapMode {
			none,
			whitespace,
			endOfLine
		};

		struct Cursor {
			U32 row;
			U32 col;
			U32 colStart;
			U32 colEnd;
		};

		virtual void set_char(U32 row, U32 col, U32 foreground, U32 background, U8) = 0;
		virtual auto get_char(U32 row, U32 col) -> U8 = 0;
		virtual auto get_char_foreground(U32 row, U32 col) -> U32 = 0;
		virtual auto get_char_background(U32 row, U32 col) -> U32 = 0;

		/*   */ void write_text(U32 foreground, U32 background, const char *s) override { write_text(consoleOutCursor, foreground, background, WrapMode::endOfLine, s, true); }
		virtual void write_text(Cursor&, U32 foreground, U32 background, WrapMode wrapMode, const char *, bool autoScroll);

		virtual void clear();
		/*   */ void clear(U32 foreground, U32 background) override { return clear(foreground, background, ' '); }
		virtual void clear(U32 foreground, U32 background, char bgChar);
		virtual void fill(U32 row, U32 col, U32 rowCount, U32 colCount, U32 foreground, U32 background, U8 bgChar = ' ');
		virtual void scroll(I32 rows, I32 cols, U32 FgColour, U32 background, U8 bgChar = ' ');
		virtual void scroll_region(U32 startRow, U32 startCol, U32 rows, U32 cols, I32 scrollRows, I32 scrollCols, U32 FgColour, U32 background, U8 bgChar = ' ');

		auto _on_start() -> Try<> override;

		Textmode::Cursor consoleOutCursor;
	};
}
