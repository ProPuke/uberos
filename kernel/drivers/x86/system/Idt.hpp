#pragma once

#include <kernel/drivers/Hardware.hpp>

namespace driver::system {
	struct Idt final: Hardware {
		DRIVER_INSTANCE(Idt, "idt", "Interrupt Descriptor Table", Hardware)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto can_stop_driver() -> bool override { return false; }
		auto can_restart_driver() -> bool override { return false; }

		void set_entry(U8 i, void *isr, U8 flags);
		void apply_entries();
	};
}
