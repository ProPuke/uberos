#pragma once

#include <kernel/Thread.hpp>

inline void __attribute__((flatten)) Thread::swap_state(Thread &from, Thread &to) {
	asm volatile(
		"adr   r12, 1f\n"
		"push  {r12}\n"
		"push  {lr}\n"
		"mrs   r12, cpsr\n"
		"push  {r0-r12}\n"
		"str   sp, [%0]\n"
		
		"ldr   sp, [%1]\n"
		"pop   {r0-r12}\n"
		"msr   cpsr_c, r12\n"
		"pop   {lr}\n"
		"pop   {pc}\n"

		"1:"

		:
		: "r" (&from.storedState),
		  "r" (&to.storedState)
	);
}
