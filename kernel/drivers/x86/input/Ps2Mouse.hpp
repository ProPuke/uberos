#pragma once

#include <kernel/drivers/Mouse.hpp>

#include <common/Try.hpp>

namespace driver::system {
	struct Ps2;
}

namespace driver::input {
	struct Ps2Mouse final: Mouse {
		DRIVER_INSTANCE(Ps2Mouse, 0x65954e1e, "ps2Mouse", "PS/2 Mouse Controller", Mouse)

		enum struct Ps2Command {
			setScale1 = 0xe6,
			setScale2 = 0xe7,
			setResolution = 0xe8,
			read = 0xeb,
			identify = 0xf2,
			sampleRate = 0xf3, //10, 20, 40, 60, 80, 100 or 200
			enableReporting = 0xf4,
			disableReporting = 0xf5,
			setdefaults = 0xf6,
			reset = 0xff
		};

		enum struct Ps2Response {
			ack = 0xfa
		};

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;
		void _on_irq(U8) override;

		auto get_position_x() -> I32;
		auto get_position_y() -> I32;
		auto get_button_count() -> U8;
		auto get_button_state(U8) -> bool;

		void send_ps2_command(Ps2Command);
		void send_ps2_data(U8);
	};
}
