#pragma once

#include <kernel/drivers/raspi/timer/Raspi_bcm.hpp>

namespace arch {
	namespace raspi {
		namespace timer {
			extern driver::timer::Raspi_bcm bcm_timer;

			void init();
		}
	}
}
