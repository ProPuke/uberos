#include <kernel/drivers/raspi/processor/Raspi.hpp>

#include <common/Try.hpp>

namespace driver::processor {
	struct Raspi_bcm2836 final: Raspi {
		DRIVER_INSTANCE(Raspi_bcm2836, "bcm2836", "BCM2836 AArch32 Processor", Raspi)

		auto _on_start() -> Try<> override {
			processor_arch = "AArch32";
			processor_cores = 4;
			return {};
		}
	};
}
