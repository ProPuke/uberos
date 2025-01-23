#pragma once

#include <kernel/drivers/Textmode.hpp>

#include <common/Try.hpp>

namespace driver::textmode {
	struct VgaTextmode final: driver::Textmode {
		DRIVER_INSTANCE(VgaTextmode, "vgaText", "VGA Textmode", driver::Textmode)

		struct __attribute__((packed)) Entry {
			U8 c;
			U8 fgColour:4;
			U8 bgColour:4;
		};

		U64 _address = 0xb8000;

		auto get_mode_count() -> U32 override;
		auto get_mode(U32 mode) -> Mode override;
		void set_mode(U32 mode) override;

		auto _on_start() -> Try<>;
		auto _on_stop() -> Try<>;

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
