#include <common/config.h>

#ifdef _64BIT
	.section .init, "ax", @progbits
		mov rsp, rbp
		pop rbp
		ret

	.section .fini, "ax", @progbits
		mov rsp, rbp
		pop rbp
		ret
#else
	.section .init, "ax", @progbits
		pop ebp
		ret

	.section .fini, "ax", @progbits
		pop ebp
		ret
#endif
