#pragma once

#include <kernel/drivers/Console.hpp>

#include <common/Try.hpp>

namespace driver::console {
	struct UefiConsole: driver::Console {
		DRIVER_INSTANCE(UefiConsole, 0x10d47305, "uefiConsole", "UEFI Simple Text Output", driver::Console)

		auto get_mode_count() -> U32 override;
		auto get_mode(U32 mode) -> Mode override;
		void set_mode(U32 mode) override;

		auto get_rows() -> U32 override;
		auto get_cols() -> U32 override;

		auto get_colour_count() -> U32 override;
		auto get_colour(U32) -> U32 override;

		void write_text(U32 foreground, U32 background, const char*) override;

		void clear(U32 foreground, U32 background) override;

		auto _on_start() -> Try<> override;

	};
}
