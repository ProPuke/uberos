#include <kernel/driver/Irq.hpp>

namespace driver {
	namespace irq {
		struct Arm_gicV2: driver::Irq {
			/**/ Arm_gicV2(U32 address):
				Irq(address, "GIC v2", "interrupt controller")
			{}

			void _on_driver_enable() override;
			void _on_driver_disable() override;
			
			void enable_irq(U32 irq, U8 cpu) override;
			void disable_irq(U32 irq) override;
		};
	}
}
