#include <kernel/driver/Interrupt.hpp>

namespace driver {
	namespace interrupt {
		struct Arm_raspi_legacy: driver::Interrupt {
			/**/ Arm_raspi_legacy(U32 address);

			void _on_driver_enable() override;
			void _on_driver_disable() override;
			
			void enable_irq(U32 cpu, U32 irq) override;
			void disable_irq(U32 cpu, U32 irq) override;

			void handle_interrupt(InterruptHandler callback) override;
		};
	}
}
