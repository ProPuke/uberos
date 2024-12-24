#pragma once

#include <kernel/drivers/Interrupt.hpp>

namespace driver {
	namespace interrupt {
		struct Pic8259 final: driver::Interrupt {
			typedef driver::Interrupt Super;

			U8 irqOffset[2];

			/**/ Pic8259(const char *name = "8259 PIC");

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;

			void enable_irq(U32 cpu, U32 irq) override;
			void disable_irq(U32 cpu, U32 irq) override;
			void disable_all_irqs();

			void set_offset(U8 offset1, U8 offset2);

			auto _on_interrupt(void *_cpuState) -> const void*;

		protected:
		};
	}
}
