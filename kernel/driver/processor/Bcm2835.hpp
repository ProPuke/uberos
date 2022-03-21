#include <kernel/driver/Processor.hpp>

namespace driver {
	namespace processor {
		struct Bcm2835: driver::Processor {
			/**/ Bcm2835():
				Processor("BCM2835", "aarch32", "cpu")
			{
				is_builtin = true;
				processor_cores = 1;
			}
		};
	}
}
