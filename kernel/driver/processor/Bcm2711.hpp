#include <kernel/driver/Processor.hpp>

namespace driver {
	namespace processor {
		struct Bcm2711: driver::Processor {
			/**/ Bcm2711():
				Processor("BCM2711", "aarch64", "cpu")
			{
				is_builtin = true;
				processor_cores = 4;
			}
		};
	}
}
