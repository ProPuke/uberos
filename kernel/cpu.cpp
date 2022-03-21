#include "cpu.hpp"

#include <kernel/deviceManager.hpp>

#if defined(ARCH_RASPI1)
	#include <kernel/driver/processor/Bcm2835.hpp>

#elif defined(ARCH_RASPI2)
	#include <kernel/driver/processor/Bcm2836.hpp>

#elif defined(ARCH_RASPI3)
	#include <kernel/driver/processor/Bcm2837.hpp>

#elif defined(ARCH_RASPI4)
	#include <kernel/driver/processor/Bcm2711.hpp>
#endif

namespace cpu {
	#if defined(ARCH_RASPI1)
		driver::processor::Bcm2835 device;

	#elif defined(ARCH_RASPI2)
		driver::processor::Bcm2836 device;

	#elif defined(ARCH_RASPI3)
		driver::processor::Bcm2837 device;

	#elif defined(ARCH_RASPI4)
		driver::processor::Bcm2711 device;
	#endif

	void init() {
		deviceManager::add_device(device);
	}
}
