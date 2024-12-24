#pragma once

#include <kernel/Driver.hpp>
#include <kernel/framebuffer.hpp>
#include <kernel/Framebuffer.hpp>

#include <common/graphics2d/BufferFormat.hpp>

namespace driver {
	struct Textmode: Driver {
		typedef Driver Super;

		static DriverType driverType;

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

		/**/ Textmode(const char *name, const char *description);

		I32 ansiColours[8];
		I32 brightAnsiColours[8];

		virtual auto get_rows() -> U32 = 0;
		virtual auto get_cols() -> U32 = 0;

		virtual auto get_colour_count() -> U32 = 0;
		virtual auto get_colour(U32) -> U32 = 0;
		virtual auto get_nearest_colour(U32 colour) -> U32;

		virtual void set_char(U32 row, U32 col, U32 foreground, U32 background, U8) = 0;
		virtual auto get_char(U32 row, U32 col) -> U8 = 0;
		virtual auto get_char_foreground(U32 row, U32 col) -> U32 = 0;
		virtual auto get_char_background(U32 row, U32 col) -> U32 = 0;

		virtual void write_text(Cursor&, U32 foreground, U32 background, WrapMode wrapMode, const char *, bool autoScroll);

		virtual void clear(U32 foreground, U32 background, char bgChar = ' ');
		virtual void fill(U32 row, U32 col, U32 rowCount, U32 colCount, U32 foreground, U32 background, U8 bgChar = ' ');
		virtual void scroll(I32 rows, I32 cols, U32 FgColour, U32 background, U8 bgChar = ' ');
		virtual void scroll_region(U32 startRow, U32 startCol, U32 rows, U32 cols, I32 scrollRows, I32 scrollCols, U32 FgColour, U32 background, U8 bgChar = ' ');

		void bind_to_console();

		auto _on_start() -> bool override;

		struct ConsoleContext {
			Textmode::Cursor cursor;
			U32 defaultForeground;
			U32 defaultBackground;
			U32 foreground;
			U32 background;
			bool ansiBold = false;

			char currentEscape[64] = "";
			unsigned currentEscapeLength = 0;
		} consoleContext;
	};
}
