#include <kernel/Thread.hpp>

void Thread::swap_state(Thread &from, Thread &to) {
	asm volatile(
		"mrs   r12, cpsr\n"
		"push  {lr, sp}\n" //put the current lr into pc, and sp (rubbish) into lr (lr doesn't matter, as the current lr will restore it when we return)
		"push  {r4-r12}\n"
		"str   sp, [%0]\n"
		
		"ldr   sp, [%1]\n"
		"pop   {r4-r12}\n"
		"msr   cpsr_c, r12\n"
		"pop   {lr}\n"
		"pop   {pc}\n"

		:
		: "r" (&from.storedState),
		  "r" (&to.storedState)
	);
}
