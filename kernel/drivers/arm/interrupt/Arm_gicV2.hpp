#include <kernel/drivers/Interrupt.hpp>

#include <common/Try.hpp>

namespace driver::interrupt {
	struct Arm_gicV2: driver::Interrupt {
		DRIVER_TYPE_CUSTOM_CTOR(Arm_gicV2, "gic2", "GIC v2", driver::Interrupt)

		/**/ Arm_gicV2(U32 address):
			Super(DriverApi::Startup::automatic),
			_address(address)
		{ DRIVER_DECLARE_INIT(); }

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		U32 _address;
		
		void enable_irq(U32 cpu, U32 irq) override;
		void disable_irq(U32 cpu, U32 irq) override;
		// auto get_active_interrupt(U32 cpu) -> U32 override;

		auto handle_interrupt(const void *cpuState) -> const void* override;
	};
}

