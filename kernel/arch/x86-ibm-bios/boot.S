#include <kernel/kernel.h>

.code32

.section ".text.boot"
	.global _start
	.type _start, @function
	_start:
		// set stack pointer
			lea esp, __end
			add esp, KERNEL_STACK_SIZE

		// clear the bss section
			push eax
			lea edi, [__bss_start]
			lea ecx, [__bss_end]
			sub ecx, edi
			shr ecx, 2 // divide by 4 for dword count
			xor eax, eax
			rep stosd
			pop eax

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

