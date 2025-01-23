#include <kernel/drivers/Interrupt.hpp>

#include <common/Try.hpp>

namespace driver::interrupt {
	struct Arm_raspi_legacy final: driver::Interrupt {
		DRIVER_INSTANCE(Arm_raspi_legacy, "raspiIrq", "Raspi Legacy Interrupt Controller", driver::Interrupt)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		void enable_irq(U32 cpu, U32 irq) override;
		void disable_irq(U32 cpu, U32 irq) override;

		auto handle_interrupt(const void *cpuState) -> const void* override;
	};
}
