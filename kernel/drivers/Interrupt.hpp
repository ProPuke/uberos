#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	struct Interrupt: Driver {
		typedef Driver Super;

		static DriverType driverType;

		U32 min_irq = 0;
		U32 max_irq = 0;

		typedef void(*InterruptHandler)(U32 interrupt);

		/**/ Interrupt(const char *name, const char *description);

		virtual void enable_irq(U32 cpu, U32 irq) = 0; // set this irq as active, and make sure it forwards to at least JUST this cpu. It may also forward to other cpus if it doesn't support per-cpu irqs. This replaces previous settings if this irq was already enabled
		virtual void disable_irq(U32 cpu, U32 irq) = 0;
		// virtual auto get_active_interrupt(U32 cpu) -> U32 = 0;
	};
}
