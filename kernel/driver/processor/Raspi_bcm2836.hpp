#include "Raspi.hpp"

namespace driver {
	namespace processor {
		struct Raspi_bcm2836: Raspi {
			/**/ Raspi_bcm2836():
				Raspi("BCM2836", "aarch32")
			{
				processor_cores = 4;
			}
		};
	}
}
