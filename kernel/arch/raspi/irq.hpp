#pragma once

namespace irq {
	namespace arch {
		namespace raspi {
			enum struct Irq {
				system_timer_0 = 0,
				system_timer_1 = 1,
				system_timer_2 = 2,
				system_timer_3 = 3,
				usb_controller = 9,
				arm_timer = 64
			};

			inline const unsigned irq_max = 72;

			void init();
			void enable(Irq irq);
			void disable(Irq irq);
		}
	}
}
