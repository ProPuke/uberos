#pragma once

#include <kernel/drivers/Hardware.hpp>

#include <common/Bitmask.hpp>
#include <common/Try.hpp>

namespace driver {
	struct Interrupt: Hardware {
		DRIVER_TYPE(Interrupt, "Interrupt", "Interrupt Controller", Hardware);

		U32 min_irq = 0;
		U32 max_irq = 0;

		typedef void(*InterruptHandler)(U32 interrupt);

		virtual void enable_irq(U32 cpu, U8 irq) = 0; // set this irq as active, and make sure it forwards to at least JUST this cpu. It may also forward to other cpus if it doesn't support per-cpu irqs. This replaces previous settings if this irq was already enabled
		virtual void disable_irq(U32 cpu, U8 irq) = 0;
		// virtual auto get_active_interrupt(U32 cpu) -> U32 = 0;
		virtual auto get_available_irq(Bitmask256) -> Try<U8> = 0;
	};
}
