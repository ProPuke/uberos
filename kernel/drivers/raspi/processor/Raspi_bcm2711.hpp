#include <kernel/drivers/raspi/processor/Raspi.hpp>

namespace driver {
	namespace processor {
		struct Raspi_bcm2711: Raspi {
			/**/ Raspi_bcm2711():
				Raspi("BCM2711", "aarch64")
			{
				processor_cores = 4;
			}

			auto _on_start() -> bool override { return true; }
		};
	}
}
