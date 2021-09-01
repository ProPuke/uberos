#include <kernel/Thread.hpp>

void Thread::swap_state(Thread &from, Thread &to) {
	//x29 is fp
	//x30 is lr

	asm volatile(
		"sub sp, sp, #16*7\n"
		"mov x8, sp\n"
		"str x8, [%0]\n"
		"stp x19, x20, [sp, #16* 0]\n"
		"stp x21, x22, [sp, #16* 1]\n"
		"stp x23, x24, [sp, #16* 2]\n"
		"stp x25, x26, [sp, #16* 3]\n"
		"stp x27, x28, [sp, #16* 4]\n"
		"stp x29, xzr, [sp, #16* 5]\n" // framepointer and zero into lr (as this will get overwritten on return)
		"str x30,      [sp, #16* 6]\n" // lr as pc
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
		"ldp x29, x30, [sp, #16* 5]\n" //framepointer and lr
		"ldr  x8,      [sp, #16* 6]\n" //pc
		// "ldp q14, q15, [sp, #32* 6]\n"
		// "ldp q12, q13, [sp, #32* 8]\n"
		// "ldp q10, q11, [sp, #32*10]\n"
		"add sp, sp, #16*7\n"
		"br  x8\n" //ret to make lr pc

		:
		: "r" (&from.storedState),
		  "r" (&to.storedState)
	);
}
