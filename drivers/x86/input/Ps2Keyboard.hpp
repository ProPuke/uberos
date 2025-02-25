#pragma once

#include <drivers/Keyboard.hpp>

#include <common/Try.hpp>

namespace driver::input {
	struct Ps2Keyboard final: Keyboard {
		DRIVER_INSTANCE(Ps2Keyboard, 0xeceda7cc, "ps2Keyboard", "PS/2 Keyboard Controller", Keyboard)

		enum struct Ps2Command {
			setLeds = 0xed,
			scancode = 0xf0,
			setRepeat = 0xf3,
			enableReporting = 0xf4,
			disableReporting = 0xf5,
			reset = 0xff,
		};

		enum struct Ps2CommandScancode {
			get,
			set1,
			set2,
			set3
		};

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;
		void _on_irq(U8) override;

		auto is_pressed(keyboard::Scancode) -> bool override;

		void send_ps2_command(Ps2Command);
		void send_ps2_data(U8);
	};
}
