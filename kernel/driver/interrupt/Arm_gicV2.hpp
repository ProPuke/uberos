#include <kernel/driver/Interrupt.hpp>

namespace driver {
	namespace interrupt {
		struct Arm_gicV2: driver::Interrupt {
			/**/ Arm_gicV2(U32 address):
				Interrupt(address, "GIC v2", "interrupt controller")
			{}

			void _on_driver_enable() override;
			void _on_driver_disable() override;
			
			void enable_irq(U32 cpu, U32 irq) override;
			void disable_irq(U32 cpu, U32 irq) override;
			// auto get_active_interrupt(U32 cpu) -> U32 override;

			void handle_interrupt(InterruptHandler callback) override;
		};
	}
}
