#pragma once

#include <drivers/raspi/processor/Raspi.hpp>

#include <common/Try.hpp>

namespace driver::processor {
	struct Raspi_bcm2836 final: Raspi {
		DRIVER_INSTANCE(Raspi_bcm2836, 0x9ca0d34e, "bcm2836", "BCM2836 AArch32 Processor", Raspi)

		auto _on_start() -> Try<> override {
			if(::processor::driver&&::processor::driver!=this) return {"A CPU driver is already active"};

			processor_arch = "AArch32";
			processor_cores = 4;

			::processor::driver = this;

			return {};
		}
	};
}
