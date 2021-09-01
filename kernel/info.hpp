#pragma once

namespace info {
	static const char *cpu_arch = 
		#if defined(ARCH_ARM32)
			"ARM32"
		#elif defined(ARCH_ARM64)
			"ARM64"
		#elif defined(ARCH_X64)
			"x64"
		#elif defined(ARCH_X86)
			"x86"
		#else
			#error "Unknown architecture"
		#endif
	;
	static const char *device_type = 
		#if defined(ARCH_RASPI)
			"Raspberry Pi"
		#elif defined(ARCH_X64) or defined(ARCH_X86)
			"IBM-Compatible PC"
		#else
			#error "Unknown device type"
		#endif
	;
	static const char *device_version = 
		#if defined(ARCH_RASPI1)
			"1"
		#elif defined(ARCH_RASPI2)
			"2"
		#elif defined(ARCH_RASPI3)
			"3"
		#elif defined(ARCH_RASPI4)
			"4"
		#else
			#error "Unknown device version"
		#endif
	;
}
