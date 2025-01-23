#pragma once

#include <kernel/ConsoleOut.hpp>
#include <kernel/drivers/Hardware.hpp>

#include <common/Try.hpp>

namespace driver {
	struct Console: Hardware {
		DRIVER_TYPE(Console, "console", "Console Output Driver", Hardware);

		struct Mode {
			U32 rows;
			U32 cols;
			U32 colours;
		};

		virtual auto get_mode_count() -> U32 { return 0; }
		virtual auto get_mode(U32 mode) -> Mode { return Mode{0,0,1}; }
		virtual void set_mode(U32 mode) {;}

		virtual auto get_rows() -> U32 { return 0; } // 0 for unknown
		virtual auto get_cols() -> U32 { return 0; } // 0 for unknown

		virtual auto get_colour_count() -> U32 = 0;
		virtual auto get_colour(U32) -> U32 = 0;
		virtual auto get_nearest_colour(U32 colour) -> U32;

		virtual void write_text(U32 foreground, U32 background, const char*) = 0;

		virtual void clear(U32 foreground, U32 background) = 0;

		void bind_to_console();

		auto _on_start() -> Try<> override;

		struct ConsoleOut: ::ConsoleOut {
			auto get_colour_count() -> U32 override { return _console().get_colour_count(); }
			auto get_colour(U32 i) -> U32 override { return _console().get_colour(i); }
			auto get_nearest_colour(U32 c) -> U32 override { return _console().get_nearest_colour(c); }

			void clear() override;

			void write_raw_text(const char *s) override { _console().write_text(foregroundColour, backgroundColour, s); }

			auto _console() -> Console& { return *(Console*)((U8*)this-offsetof(Console, consoleOut)); }
		} consoleOut;
	};
}
