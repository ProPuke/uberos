#include "cpu.hpp"

#include <kernel/device.hpp>

#if defined(ARCH_RASPI1)
	#include <kernel/driver/processor/Raspi_bcm2835.hpp>

#elif defined(ARCH_RASPI2)
	#include <kernel/driver/processor/Raspi_bcm2836.hpp>

#elif defined(ARCH_RASPI3)
	#include <kernel/driver/processor/Raspi_bcm2837.hpp>

#elif defined(ARCH_RASPI4)
	#include <kernel/driver/processor/Raspi_bcm2711.hpp>
#endif

namespace cpu {
	#if defined(ARCH_RASPI1)
		driver::processor::Raspi_bcm2835 device;

	#elif defined(ARCH_RASPI2)
		driver::processor::Raspi_bcm2836 device;

	#elif defined(ARCH_RASPI3)
		driver::processor::Raspi_bcm2837 device;

	#elif defined(ARCH_RASPI4)
		driver::processor::Raspi_bcm2711 device;
	#endif

	void init() {
		device::install_device(device, true);
	}
}
