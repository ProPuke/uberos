#include <kernel/driver/Irq.hpp>

namespace driver {
	namespace irq {
		struct Arm_gic400: driver::Irq {
			/**/ Arm_gic400(U32 address):
				Irq(address, "GIC-400", "interrupt controller")
			{
				is_builtin = true;
			}

			void enable_driver() override;
			void disable_driver() override;
			
			void enable_irq(U32 irq, U8 cpu) override;
			void disable_irq(U32 irq) override;
		};
	}
}
