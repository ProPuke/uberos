#include <kernel/drivers/Interrupt.hpp>

namespace driver {
	namespace interrupt {
		struct Arm_raspi_legacy: driver::Interrupt {
			typedef driver::Interrupt Super;

			/**/ Arm_raspi_legacy(U32 address);

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;
			
			void enable_irq(U32 cpu, U32 irq) override;
			void disable_irq(U32 cpu, U32 irq) override;

			auto handle_interrupt(const void *cpuState) -> const void* override;
		};
	}
}
