#pragma once

#include <kernel/drivers/Textmode.hpp>

namespace driver {
	namespace textmode {
		struct VgaTextmode: Textmode {
			typedef Textmode Super;

			struct __attribute__((packed)) Entry {
				U8 c;
				U8 fgColour:4;
				U8 bgColour:4;
			};

			/**/ VgaTextmode();
			/**/ VgaTextmode(U64 address);

			U64 _address;

			auto _on_start() -> bool;
			auto _on_stop() -> bool;

			auto get_rows() -> U32 override;
			auto get_cols() -> U32 override;

			auto get_colour_count() -> U32 override;
			auto get_colour(U32) -> U32 override;

			void set_char(U32 row, U32 col, U32 foreground, U32 background, U8) override;
			auto get_char(U32 row, U32 col) -> U8 override;
			auto get_char_foreground(U32 row, U32 col) -> U32 override;
			auto get_char_background(U32 row, U32 col) -> U32 override;

			auto get_entry(U32 row, U32 col) -> Entry&;
			auto buffer() -> Entry*;
		};
	}
}
