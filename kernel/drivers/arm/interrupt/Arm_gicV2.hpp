#include <kernel/drivers/Interrupt.hpp>

namespace driver {
	namespace interrupt {
		struct Arm_gicV2: driver::Interrupt {
			/**/ Arm_gicV2(U32 address):
				Interrupt(address, "GIC v2", "interrupt controller")
			{}

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;
			
			void enable_irq(U32 cpu, U32 irq) override;
			void disable_irq(U32 cpu, U32 irq) override;
			// auto get_active_interrupt(U32 cpu) -> U32 override;

			auto handle_interrupt(const void *cpuState) -> const void* override;
		};
	}
}
