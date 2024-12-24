#include <kernel/kernel.h>

.section ".text.boot"

.global _start

// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS

_start:

	//halt additional cores
		#ifndef ARCH_RASPI1
			mrc p15, #0, r1, c0, c0, #5
			and r1, r1, #3
			cmp r1, #0
			bne halt
		#endif

	//set stack pointer
		ldr sp, = __end
		mov r4, #(KERNEL_STACK_SIZE)
		add sp, sp, r4

	//clear the bss section
		ldr r4, = __bss_start
		ldr r9, = __bss_end
		mov r5, #0
		mov r6, #0
		mov r7, #0
		mov r8, #0
		
		1:
		cmp r4, r9
		bhs 2f
		stmia r4!, {r5-r8}
		b 1b
		2:

	//call entrypoint
		mov r0, #0x100 //atags
		bl entrypoint
		b halt

halt:
	wfe
	b halt
