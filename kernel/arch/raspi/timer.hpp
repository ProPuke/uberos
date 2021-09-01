#pragma once

#include <kernel/timer.hpp>
#include <common/types.hpp>

namespace timer {
	namespace arch {
		namespace raspi {
			enum struct Timer: U8 {
				gpu0,
				cpu_scheduler,
				gpu1,
				cpu_slow_scheduler
			};

			void init();

			void set_timer(Timer timer, U32 usecs);
			
			void on_interrupt(Timer timer);
		}
	}
}
