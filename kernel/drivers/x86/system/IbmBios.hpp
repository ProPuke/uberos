#pragma once

#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/drivers/Hardware.hpp>

namespace driver::system {
	struct IbmBios final: Hardware {
		DRIVER_INSTANCE(IbmBios, 0x8c9fb94d, "ibmbios", "IBM-Compatible BIOS", Hardware)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		enum IoPort {
			com1,
			com2,
			com3,
			com4,
			lpt1,
			lpt2,
			lpt3
		};

		enum VideoType {
			none,
			monochrome80,
			colour40,
			colour80
		};

		auto get_io(IoPort) -> arch::x86::IoPort;
		auto get_video_type() -> VideoType;
		auto get_possible_ebda() -> void*;

		void set_mode(U32);
	};
}
