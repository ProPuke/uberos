#include <kernel/drivers/raspi/processor/Raspi.hpp>

namespace driver {
	namespace processor {
		struct Raspi_bcm2836: Raspi {
			/**/ Raspi_bcm2836():
				Raspi("BCM2836", "aarch32")
			{
				processor_cores = 4;
			}

			auto _on_start() -> bool override { return true; }
		};
	}
}
