#pragma once

#include <drivers/Hardware.hpp>
#include <drivers/ResidentService.hpp>

namespace driver::system {
	struct Idt final: ResidentService<Hardware> {
		DRIVER_INSTANCE(Idt, 0xeca193f8, "idt", "Interrupt Descriptor Table", ResidentService<Hardware>)

		auto _on_start() -> Try<> override;

		void set_gate_trap(U8 interrupt, void *isr);
		void set_gate_interrupt(U8 interrupt, void *isr);
		void apply_gates();
	};
}
