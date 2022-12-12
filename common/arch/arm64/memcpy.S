#include <common/stdlib.h>

#ifdef USE_STDLIB_ASM
	#include <lib/arm-optimized-routines/string/aarch64/memcpy.S> //also includes memmove
	// #include <lib/arm-optimized-routines/string/aarch64/memcpy-advsimd.S>
#endif
