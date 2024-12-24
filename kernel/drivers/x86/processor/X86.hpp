#include <kernel/drivers/Processor.hpp>

namespace driver {
	namespace processor {
		struct X86: Processor {
			typedef Processor Super;

			/**/ X86();

			char vendorStringData[13] = "UNKNOWN";

			auto _on_start() -> bool override;
		};
	}
}
