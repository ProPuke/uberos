#include <kernel/drivers/Processor.hpp>

#include <common/Try.hpp>

namespace driver::processor {
	struct X86 final: driver::Processor {
		DRIVER_INSTANCE(X86, "x86", "x86 Processor", driver::Processor)

		char vendorStringData[13] = "UNKNOWN";

		auto _on_start() -> Try<> override;
	};
}
