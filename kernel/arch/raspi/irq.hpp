#pragma once

#if defined(ARCH_RASPI4)
	#define HAS_GIC400
	#include <drivers/arm/interrupt/Arm_gicV2.hpp>
#endif
#include <drivers/raspi/interrupt/Arm_raspi_legacy.hpp>

#include <kernel/mmio.hpp>

#include <kernel/arch/raspi/mmio.hpp>

namespace arch {
	namespace raspi {
		namespace irq {
			enum struct Irq {
				system_timer_gpu_0 = 0,
				system_timer_cpu_0 = 1,
				system_timer_gpu_1 = 2,
				system_timer_cpu_1 = 3,
				usb_controller = 9,
				hdmi_0 = 40,
				hdmi_1 = 41,
				arm_timer = 64
			};

			inline const unsigned irq_max = 72;

			#ifdef HAS_GIC400
				extern driver::interrupt::Arm_gicV2 interruptController;
			#endif
			extern driver::interrupt::Arm_raspi_legacy cpuInterruptController;

			void init();
			void enable(Irq irq);
			void disable(Irq irq);
		}
	}
}
