#pragma once

#include <drivers/Processor.hpp>

#include <common/Try.hpp>

namespace driver::processor {
	struct X86 final: driver::Processor {
		DRIVER_INSTANCE(X86, 0xf8fde7a4, "x86", "x86 Processor", driver::Processor)

		char vendorStringData[13] = "UNKNOWN";

		auto _on_start() -> Try<> override;

		auto get_active_id() -> U32 override;
	};
}
