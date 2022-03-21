#include <kernel/driver/Processor.hpp>

namespace driver {
	namespace processor {
		struct Bcm2837: driver::Processor {
			/**/ Bcm2837():
				Processor("BCM2837", "aarch64", "cpu")
			{
				is_builtin = true;
				processor_cores = 4;
			}
		};
	}
}
