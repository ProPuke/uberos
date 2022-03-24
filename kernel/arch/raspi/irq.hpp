#pragma once

#if defined(ARCH_RASPI4)
	#define HAS_GIC400
	#include <kernel/driver/irq/Arm_gicV2.hpp>
#endif

#include <kernel/mmio.hpp>

#include <kernel/arch/raspi/mmio.hpp>

namespace mmio {
	using namespace arch::raspi;
}

namespace irq {
	namespace arch {
		namespace raspi {
			enum struct Irq {
				system_timer_0 = 0,
				system_timer_1 = 1,
				system_timer_2 = 2,
				system_timer_3 = 3,
				usb_controller = 9,
				hdmi_0 = 40,
				hdmi_1 = 41,
				arm_timer = 64
			};

			inline const unsigned irq_max = 72;

			#ifdef HAS_GIC400
				inline driver::irq::Arm_gicV2 interruptController {(U32)mmio::Address::gic400};
			#endif

			void init();
			void enable(Irq irq);
			void disable(Irq irq);
		}
	}
}
