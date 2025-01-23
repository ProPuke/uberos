#include <kernel/kernel.h>

.code32

.section ".text.boot"
	.global _start
	.type _start, @function
	_start:
		// set stack pointer
			lea esp, __end
			add esp, KERNEL_STACK_SIZE

		// call entrypoint
			#ifndef _64BIT
				push ebx // push multiboot info
				push eax // push multiboot magic
			#endif
			call entrypoint
			jmp halt

	halt:
		// disable interrupts
			cli
		1:
			hlt
			jmp 1b

