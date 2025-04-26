#pragma once

#include <drivers/raspi/processor/Raspi.hpp>

#include <common/Try.hpp>

namespace driver::processor {
	struct Raspi_bcm2835 final: Raspi {
		DRIVER_INSTANCE(Raspi_bcm2835, 0x8e926484, "bcm2835", "BCM2835 AArch32 Processor", Raspi)

		auto _on_start() -> Try<> override {
			if(::processor::driver&&::processor::driver!=this) return Failure{"A CPU driver is already active"};

			processor_arch = "AArch32";
			processor_cores = 1;

			::processor::driver = this;

			return {};
		}
	};
}
