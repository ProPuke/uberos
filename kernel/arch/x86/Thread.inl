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
		push ebp
		push ebx
		push esi
		push edi
		push esp
		push offset 1f // eip
		mov [%0], esp

		1:

		mov esp, [%1]
		pop eax // eip
		pop esp
		pop edi
		pop esi
		pop ebx
		pop ebp
		jmp eax
	)"
		:
		: "r" (&from.storedState),
		  "r" (&to.storedState)
	);
}
