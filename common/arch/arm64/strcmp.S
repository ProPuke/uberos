#include <common/stdlib.h>

#ifdef HAS_UNALIGNED_ACCESS
	#include <lib/arm-optimized-routines/string/aarch64/strcmp.S>
#endif
