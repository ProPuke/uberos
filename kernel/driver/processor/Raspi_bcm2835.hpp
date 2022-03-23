#include "Raspi.hpp"

namespace driver {
	namespace processor {
		struct Raspi_bcm2835: Raspi {
			/**/ Raspi_bcm2835():
				Raspi("BCM2835", "aarch32")
			{
				processor_cores = 1;
			}
		};
	}
}
