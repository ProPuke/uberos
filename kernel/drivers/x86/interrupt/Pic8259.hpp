#pragma once

#include <kernel/drivers/Interrupt.hpp>

namespace driver::interrupt {
	struct Pic8259 final: driver::Interrupt {
		DRIVER_INSTANCE(Pic8259, "pic8259", "8259 PIC", driver::Interrupt)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		void enable_irq(U32 cpu, U8 irq) override;
		void disable_irq(U32 cpu, U8 irq) override;
		void disable_all_irqs();
		auto get_available_irq(Bitmask256) -> Try<U8> override;

		void set_offset(U8 offset1, U8 offset2);

		auto _on_interrupt(U8, const void *_cpuState) -> const void* override;
	};
}
