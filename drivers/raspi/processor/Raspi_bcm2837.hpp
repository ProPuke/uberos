#pragma once

#include <drivers/raspi/processor/Raspi.hpp>

#include <common/Try.hpp>

namespace driver::processor {
	struct Raspi_bcm2837 final: Raspi {
		DRIVER_INSTANCE(Raspi_bcm2837, 0x61a965d8, "bcm2837", "BCM2837 AArch64 Processor", Raspi)

		auto _on_start() -> Try<> override {
			if(::processor::driver&&::processor::driver!=this) return {"A CPU driver is already active"};

			processor_arch = "AArch64";
			processor_cores = 4;

			::processor::driver = this;

			return {};
		}
	};
}
