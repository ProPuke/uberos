#include "Raspi.hpp"

namespace driver {
	namespace processor {
		struct Raspi_bcm2837: Raspi {
			/**/ Raspi_bcm2837():
				Raspi("BCM2837", "aarch64")
			{
				processor_cores = 4;
			}
		};
	}
}
