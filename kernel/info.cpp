#include "info.hpp"

namespace info {
	const char *cpu_arch = 
		#if defined(ARCH_ARM32)
			"aarch32"
		#elif defined(ARCH_ARM64)
			"aarch64"
		#elif defined(ARCH_X86_64)
			"x86-64"
		#elif defined(ARCH_X86)
			"x86-32"
		#elif defined(ARCH_HOSTED_LINUX)
			"virtual"
		#else
			#error "Unknown"
		#endif
	;
	const char *device_type = 
		#if defined(ARCH_RASPI)
			"Raspberry Pi"
		#elif defined(ARCH_X86_IBM_BIOS)
			"IBM-Compatible PC"
		#elif defined(ARCH_X86)
			"Unknown x86 Device"
		#elif defined(ARCH_HOSTED_LINUX)
			"Linux virtual host"
		#else
			#error "Unknown"
		#endif
	;
	const char *device_model = 
		#if defined(ARCH_RASPI1)
			"1"
		#elif defined(ARCH_RASPI2)
			"2"
		#elif defined(ARCH_RASPI3)
			"3"
		#elif defined(ARCH_RASPI4)
			"4"
		#elif defined(ARCH_X86)
			"generic desktop"
		#elif defined(ARCH_HOSTED_LINUX)
			"virtual"
		#else
			#error "unknown"
		#endif
	;

	//TODO: look up in SMBIOS on x86
	const char *device_revision = 
		"Unknown"
	;
}
