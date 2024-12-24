#pragma once

#if defined(ARCH_ARM32)
	#define _32BIT
#elif defined(ARCH_ARM64) || defined(ARCH_X86_64)
	#define _64BIT
#elif __x86_64__ || __ppc64__
	#define _64BIT
#else
	#define _32BIT
#endif

#if defined(ARCH_ARM64)
	#define HAS_128BIT
#elif defined(ARCH_X86_64)
	// #define HAS_128BIT
#elif defined(ARCH_HOSTED_LINUX)
	#define HAS_128BIT
#endif

#ifdef HAS_MMU2
	#define HAS_UNALIGNED_ACCESS
#endif
