#pragma once

#include <kernel/Thread.hpp>

inline void __attribute__((flatten)) Thread::swap_state(Thread &from, Thread &to) {
	//x29 is fp
	//x30 is lr

	asm volatile(
		"mov x9, sp\n"
		"sub sp, sp, #16*7\n"
		"mov x8, sp\n"
		"str x8, [%0]\n"
		"stp x19, x20, [sp, #16* 0]\n"
		"stp x21, x22, [sp, #16* 1]\n"
		"stp x23, x24, [sp, #16* 2]\n"
		"stp x25, x26, [sp, #16* 3]\n"
		"stp x27, x28, [sp, #16* 4]\n"
		"stp  fp,  x9, [sp, #16* 5]\n" // fp, lr
		"adr x8, 1f\n"
		"str x8,       [sp, #16* 6]\n" // pc
		// "stp q14, q15, [sp, #32* 6]\n"
		// "stp q12, q13, [sp, #32* 8]\n"
		// "stp q10, q11, [sp, #32*10]\n"

		"ldr  x8, [%1]\n"
		"mov  sp, x8\n"
		"ldp x19, x20, [sp, #16* 0]\n"
		"ldp x21, x22, [sp, #16* 1]\n"
		"ldp x23, x24, [sp, #16* 2]\n"
		"ldp x25, x26, [sp, #16* 3]\n"
		"ldp x27, x28, [sp, #16* 4]\n"
		"ldp  fp,  lr, [sp, #16* 5]\n"
		"ldr  x8,      [sp, #16* 6]\n" // pc
		// "ldp q14, q15, [sp, #32* 6]\n"
		// "ldp q12, q13, [sp, #32* 8]\n"
		// "ldp q10, q11, [sp, #32*10]\n"
		"add sp, sp, #16*7\n"
		"br  x8\n"

		"1:"

		:
		: "r" (&from.storedState),
		  "r" (&to.storedState)
	);
}
