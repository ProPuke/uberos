#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	struct Processor: Driver {
		const char *processor_arch;
		U32 processor_cores = 1;

		constexpr /**/ Processor(const char *name, const char *processor_arch, const char *descriptiveType):
			Driver(0, name, "processor", descriptiveType),
			processor_arch(processor_arch)
		{}

		auto can_disable_driver() -> bool override { return false; }
		auto can_restart_driver() -> bool override { return false; }

		void disable_driver() override {};
		void restart_driver() override {};
	};
}
