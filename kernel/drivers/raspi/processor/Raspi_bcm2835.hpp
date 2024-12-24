#include <kernel/drivers/raspi/processor/Raspi.hpp>

namespace driver {
	namespace processor {
		struct Raspi_bcm2835: Raspi {
			/**/ Raspi_bcm2835():
				Raspi("BCM2835", "aarch32")
			{
				processor_cores = 1;
			}

			auto _on_start() -> bool override { return true; }
		};
	}
}
