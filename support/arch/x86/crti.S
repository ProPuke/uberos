#include <common/config.h>

#ifdef _64BIT
	.section .init, "ax", @progbits
	.align 4
	.global _init
	.type _init, @function
	_init:
		push rbp
		mov rbp, rsp
		and rsp, -16

	.section .fini, "ax", @progbits
	.align 4
	.global _fini
	.type _fini, @function
	_fini:
		push rbp
		movq rbp, rsp
		andq rsp, -16

#else
	.section .init, "ax", @progbits
		.global _init
		.type _init, @function
		_init:
			push ebp
			mov	ebp, esp

	.section .fini, "ax", @progbits
		.global _fini
		.type _fini, @function
		_fini:
			push ebp
			mov	ebp, esp
#endif
