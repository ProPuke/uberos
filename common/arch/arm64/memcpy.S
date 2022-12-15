#include <common/stdlib.h>

#ifdef HAS_UNALIGNED_ACCESS
	#include <lib/arm-optimized-routines/string/aarch64/memcpy.S> //also includes memmove
	// #include <lib/arm-optimized-routines/string/aarch64/memcpy-advsimd.S>
#endif
