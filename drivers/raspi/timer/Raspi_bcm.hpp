#pragma once

#include <drivers/Timer.hpp>

#include <kernel/arch/raspi/irq.hpp>
#include <kernel/arch/raspi/mmio.hpp>

namespace driver {
	namespace timer {
		struct Raspi_bcm final: driver::Timer {
			DRIVER_TYPE_CUSTOM_CTOR(Raspi_bcm, 0x647627ea, "bcm", "Raspberry Pi BCM", driver::Timer)

			/**/ Raspi_bcm(
				U64 address = (U64)arch::raspi::mmio::Address::system_timer_base,
				U32 irqAddress = (U32)arch::raspi::irq::Irq::system_timer_gpu_0,
			):
				Super(DriverApi::Startup::automatic),
				_address(address),
				irqAddress(irqAddress)
			{ DRIVER_DECLARE_INIT(); }

			enum struct Timer {
				gpu_0,
				cpu_0,
				gpu_1,
				cpu_1
			};

			U32 _address;

			void set_timer(Timer, U32 usecs);

			U32 now() override;
			U64 now64() override;
			void wait(U32 usecs) override;

			void _on_driver_start() override;
			void _on_irq(U32 irq) override;

			static void on_timer(Timer);

		protected:

			U32 irqAddress;
		};
	}
}
