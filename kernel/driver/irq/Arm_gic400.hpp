#include <kernel/driver/Irq.hpp>

namespace driver {
	namespace irq {
		struct Arm_gic400: driver::Irq {
			/**/ Arm_gic400(U32 address):
				Irq(address, "GIC-400", "interrupt controller")
			{}

			void _on_driver_enable() override;
			void _on_driver_disable() override;
			
			void enable_irq(U32 irq, U8 cpu) override;
			void disable_irq(U32 irq) override;
		};
	}
}
