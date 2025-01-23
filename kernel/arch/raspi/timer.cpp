#include "timer.hpp"

#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/CriticalSection.hpp>
#include <kernel/Log.hpp>

#include <common/types.hpp>

static Log log("arch::raspi::timer");

namespace mmio {
	using namespace arch::raspi::mmio;
}

namespace timer {
	void _schedule_update(U32 usecs);
	auto _check() -> bool;
}

namespace arch {
	namespace raspi {
		namespace timer {
			driver::timer::Raspi_bcm bcm_timer((U64)mmio::Address::uart0);

			void init() {
				drivers::install_driver(bcm_timer, true);
			}
		}
	}
}

namespace driver {
	namespace timer {
		void Raspi_bcm::on_timer(Timer timer) {
			switch(timer){
				case Timer::gpu_0: break;
				case Timer::cpu_0:
					if(!::timer::_check()){
						::timer::_schedule_update(0); // failed? try again in a bit..
					}
				break;
				case Timer::gpu_1: break;
				case Timer::cpu_1: break;
			}
		}
	}
}

namespace timer {
	void _schedule_update(U32 usecs) {
		return arch::raspi::timer::bcm_timer.set_timer(driver::timer::Raspi_bcm::Timer::cpu_0, usecs);
	}

	U32 now() {
		return arch::raspi::timer::bcm_timer.now();
	}

	U64 now64() {
		return arch::raspi::timer::bcm_timer.now64();
	}

	void wait(U32 usecs) {
		return arch::raspi::timer::bcm_timer.wait(usecs);
	}
}
