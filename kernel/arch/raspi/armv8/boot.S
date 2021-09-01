.equ KERNEL_STACK_SIZE, 4096

.section ".text.boot"

#define SCTLR_RESERVED          (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define SCTLR_EE_LITTLE_ENDIAN  (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN (0 << 24)
#define SCTLR_I_CACHE_DISABLED  (0 << 12)
#define SCTLR_D_CACHE_DISABLED  (0 << 2)
#define SCTLR_MMU_DISABLED      (0 << 0)
#define SCTLR_MMU_ENABLED       (1 << 0)

#define HCR_RW                  (1 << 31)

#define SCR_RESERVED            (3 << 4)
#define SCR_RW                  (1 << 10)
#define SCR_NS                  (1 << 0)

#define SPSR_MASK_ALL           (7 << 6)
#define SPSR_EL1h               (5 << 0)

#define CPACR_FPEN_TRAP_0    (1 << 20)
#define CPACR_FPEN_TRAP_1    (1 << 21)
#define CPACR_FPEN_TRAP_OFF  (1 << 21) | (1 << 20)

.global _start
_start:
	//halt additional cores
		mrs x1, mpidr_el1
		and x1, x1, #3
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
			ldr x0, = (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)
			msr sctlr_el1, x0

			ldr x0, = (HCR_RW)
			msr hcr_el2, x0
			
			ldr x0, = (SCR_RESERVED | SCR_RW | SCR_NS)
			msr scr_el3, x0

			ldr x0, = (SPSR_MASK_ALL | SPSR_EL1h)
			msr spsr_el3, x0

			mrs x0, cpacr_el1
			orr x0, x0, (CPACR_FPEN_TRAP_OFF)
			msr cpacr_el1, x0

			//exit into level 1
			adr x0, 1f
			msr elr_el3, x0
			eret

		// level 2
		2:
			ldr x0, = (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)
			msr sctlr_el1, x0

			mrs	x0, hcr_el2
			orr	x0, x0, (HCR_RW)
			msr	hcr_el2, x0

			mov x0, (SPSR_MASK_ALL | SPSR_EL1h)
			msr	spsr_el2, x0

			// ldr x0, = (SPSR_MASK_ALL | SPSR_EL1h)
			// msr spsr_el2, x0

			// mrs x0, cpacr_el1
			// orr x0, x0, (CPACR_FPEN_TRAP_OFF)
			// msr cpacr_el1, x0

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