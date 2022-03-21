#include <kernel/driver/Processor.hpp>

namespace driver {
	namespace processor {
		struct Bcm2836: driver::Processor {
			/**/ Bcm2836():
				Processor("BCM2836", "aarch32", "cpu")
			{
				is_builtin = true;
				processor_cores = 4;
			}
		};
	}
}
