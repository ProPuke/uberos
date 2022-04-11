#include <kernel/arch/arm64/systemRegisters.h>
#include <kernel/kernel.h>

.section ".text.boot"

.global _start
_start:
	//halt additional cores
		mrs x1, mpidr_el1
		and x1, x1, #0xff
		cbnz x1, halt

	//handle execution level
		mrs x0, CurrentEL
		lsr x0, x0, #2

		cmp x0, #3
		beq 3f

		cmp x0, #2
		beq 2f

		b 1f

		// level 3
		3:
			ldr x0, = (SCTLR_RESERVED | SCTLR_EXCEPT_END_CONTEXT_SYNC | SCTLR_EE_LITTLE_ENDIAN) & (~SCTLR_INSTRUCTION_CACHE) & (~SCTLR_DATA_CACHE) & (~SCTLR_MMU)
			msr sctlr_el1, x0

			ldr x0, = (HCR_RW)
			msr hcr_el2, x0
			
			ldr x0, = (SCR_RESERVED | SCR_RW | SCR_NS)
			msr scr_el3, x0

			ldr x0, = (SPSR_MASK_ALL | SPSR_EL2h)
			msr spsr_el3, x0

			mrs x0, cpacr_el1
			orr x0, x0, (CPACR_FPEN_TRAP_OFF)
			msr cpacr_el1, x0

			//exit into level 2
			adr x0, 2f
			msr elr_el3, x0
			eret

		// level 2
		2:
			ldr x0, = (SCTLR_RESERVED | SCTLR_EXCEPT_END_CONTEXT_SYNC | SCTLR_EE_LITTLE_ENDIAN) & (~SCTLR_INSTRUCTION_CACHE) & (~SCTLR_DATA_CACHE) & (~SCTLR_MMU)
			msr sctlr_el1, x0

			mrs	x0, hcr_el2
			orr	x0, x0, (HCR_RW)
			msr	hcr_el2, x0

			mov x0, (SPSR_MASK_ALL | SPSR_EL1h)
			msr	spsr_el2, x0

			// ldr x0, = (SPSR_MASK_ALL | SPSR_EL1h)
			// msr spsr_el2, x0

			mrs x0, cpacr_el1
			orr x0, x0, (CPACR_FPEN_TRAP_OFF)
			msr cpacr_el1, x0

			//exit into level 1
			adr x0, 1f
			msr elr_el2, x0
			eret

		// level 1
		1:
			// 👍

	//set stack pointer
		ldr x1, = __end
		mov sp, x1
		mov x1, #(KERNEL_STACK_SIZE)
		add sp, sp, x1

	//clear the bss section
		ldr x1, = __bss_start
		ldr x2, = __bss_end

		1:
		cmp x1, x2
		bhs 2f
		str xzr, [x1], #8
		add x1, x1, 8
		b 1b
		2:

	//call entrypoint
		mov x0, #0x100 //atags
		bl kernel_main
		b halt

halt:
	wfe
	b halt