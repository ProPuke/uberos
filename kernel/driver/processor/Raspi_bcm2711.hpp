#include "Raspi.hpp"

namespace driver {
	namespace processor {
		struct Raspi_bcm2711: Raspi {
			/**/ Raspi_bcm2711():
				Raspi("BCM2711", "aarch64")
			{
				processor_cores = 4;
			}
		};
	}
}
