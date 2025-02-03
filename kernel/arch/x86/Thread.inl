#pragma once

#include <kernel/Thread.hpp>

#include <kernel/mmu.hpp>

inline void __attribute__((flatten)) Thread::swap_state(Thread &from, Thread &to) {
	#ifdef HAS_MMU2
		if(to.process.memoryMapping.pageCount>0){
			mmu::set_userspace_mapping(to.process.memoryMapping);
		}else{
			mmu::set_userspace_mapping(mmu::kernelMapping);
		}
	#endif

	asm volatile(R"(
		push offset 1f // eip
		push esp
		push edi
		push esi
		push ebx
		push ebp
		mov esp, %0

		1:

		mov esp, %1
		pop ebp
		pop ebx
		pop esi
		pop edi
		pop esp
		pop eax
		jmp eax
	)"
		:
		: "r" (from.storedState),
		  "r" (to.storedState)
	);
}
