.section ".text"

.global _vectors
.global install_exception_handlers

.macro call name
	sub sp, sp, #16 * 16
	stp  x0,  x1, [sp, #16 * 0]
	stp  x2,  x3, [sp, #16 * 1]
	stp  x4,  x5, [sp, #16 * 2]
	stp  x6,  x7, [sp, #16 * 3]
	stp  x8,  x9, [sp, #16 * 4]
	stp x10, x11, [sp, #16 * 5]
	stp x12, x13, [sp, #16 * 6]
	stp x14, x15, [sp, #16 * 7]
	stp x16, x17, [sp, #16 * 8]
	stp x18, x19, [sp, #16 * 9]
	stp x20, x21, [sp, #16 *10]
	stp x22, x23, [sp, #16 *11]
	stp x24, x25, [sp, #16 *12]
	stp x26, x27, [sp, #16 *13]
	stp x28,  fp, [sp, #16 *14]
	str  lr,      [sp, #16 *15]

	bl \name

	ldp  x0,  x1, [sp, #16 * 0]
	ldp  x2,  x3, [sp, #16 * 1]
	ldp  x4,  x5, [sp, #16 * 2]
	ldp  x6,  x7, [sp, #16 * 3]
	ldp  x8,  x9, [sp, #16 * 4]
	ldp x10, x11, [sp, #16 * 5]
	ldp x12, x13, [sp, #16 * 6]
	ldp x14, x15, [sp, #16 * 7]
	ldp x16, x17, [sp, #16 * 8]
	ldp x18, x19, [sp, #16 * 9]
	ldp x20, x21, [sp, #16 *10]
	ldp x22, x23, [sp, #16 *11]
	ldp x24, x25, [sp, #16 *12]
	ldp x26, x27, [sp, #16 *13]
	ldp x28,  fp, [sp, #16 *14]
	ldr  lr,      [sp, #16 *15]
	add sp, sp, #16 * 16
.endm

.macro call_error name
	str  x0, [sp, #-16]
	ldr  x0, =exception_error_registers
	str  x1, [x0, #8]
	ldr  x1, [sp, #-16]
	str  x1, [x0, #0]
	stp  x2,  x3, [x0, #16 * 1]
	stp  x4,  x5, [x0, #16 * 2]
	stp  x6,  x7, [x0, #16 * 3]
	stp  x8,  x9, [x0, #16 * 4]
	stp x10, x11, [x0, #16 * 5]
	stp x12, x13, [x0, #16 * 6]
	stp x14, x15, [x0, #16 * 7]
	stp x16, x17, [x0, #16 * 8]
	stp x18, x19, [x0, #16 * 9]
	stp x20, x21, [x0, #16 *10]
	stp x22, x23, [x0, #16 *11]
	stp x24, x25, [x0, #16 *12]
	stp x26, x27, [x0, #16 *13]
	stp x28,  fp, [x0, #16 *14]
	str  lr,      [x0, #16 *15]

	mrs x1, elr_el1
	mrs x2, spsr_el1
	mrs x3, esr_el1
	mrs x4, far_el1
	mrs x5, sctlr_el1
	mrs x6, tcr_el1

	stp x1, x2, [x0, #16 *16]
	stp x3, x4, [x0, #16 *17]
	stp x5, x6, [x0, #16 *18]

	bl \name

	ldr  x0, =exception_error_registers
	ldp  x2,  x3, [x0, #16 * 1]
	ldp  x4,  x5, [x0, #16 * 2]
	ldp  x6,  x7, [x0, #16 * 3]
	ldp  x8,  x9, [x0, #16 * 4]
	ldp x10, x11, [x0, #16 * 5]
	ldp x12, x13, [x0, #16 * 6]
	ldp x14, x15, [x0, #16 * 7]
	ldp x16, x17, [x0, #16 * 8]
	ldp x18, x19, [x0, #16 * 9]
	ldp x20, x21, [x0, #16 *10]
	ldp x22, x23, [x0, #16 *11]
	ldp x24, x25, [x0, #16 *12]
	ldp x26, x27, [x0, #16 *13]
	ldp x28,  fp, [x0, #16 *14]
	ldr  lr,      [x0, #16 *15]
	ldp  x0,  x1, [x0, #16 * 0]
.endm

.align 11
_vectors:

	// current exception level = 0

	.align 7
	b _do_interrupt_sync_el1t     // Synchronous EL1t
	
	.align 7
	b _do_interrupt_irq_el1t      // IRQ EL1t
	
	.align 7
	b _do_interrupt_fiq_el1t      // FIQ EL1t
	
	.align 7
	b _do_interrupt_error_el1t    // Error EL1t

	// current exception level, > 0

	.align 7
	b _do_interrupt_sync_el1h     // Synchronous EL1h
	
	.align 7
	b _do_interrupt_el1_irq       // IRQ EL1h
	
	.align 7
	b _do_interrupt_fiq_el1h      // FIQ EL1h
	
	.align 7
	b _do_interrupt_error_el1h    // Error EL1h
	

	// lower exception level, at least one is AArch64

	.align 7
	b _do_interrupt_sync_el0_64   // Synchronous 64-bit EL0
	
	.align 7
	b _do_interrupt_irq_el0_64    // IRQ 64-bit EL0
	
	.align 7
	b _do_interrupt_fiq_el0_64    // FIQ 64-bit EL0
	
	.align 7
	b _do_interrupt_error_el0_64  // Error 64-bit EL0
	
	// lower exception level, all is AArch32

	.align 7
	b _do_interrupt_sync_el0_32   // Synchronous 32-bit EL0
	
	.align 7
	b _do_interrupt_irq_el0_32    // IRQ 32-bit EL0
	
	.align 7
	b _do_interrupt_fiq_el0_32    // FIQ 32-bit EL0
	
	.align 7
	b _do_interrupt_error_el0_32  // Error 32-bit EL0

_do_interrupt_sync_el1t:
	call_error interrupt_sync_el1t// Synchronous EL1t
	eret
_do_interrupt_irq_el1t:
	call interrupt_irq_el1t      // IRQ EL1t
	eret
_do_interrupt_fiq_el1t:
	call interrupt_fiq_el1t      // FIQ EL1t
	eret
_do_interrupt_error_el1t:
	call interrupt_error_el1t    // Error EL1t
	eret

_do_interrupt_sync_el1h:
	call_error interrupt_sync_el1h // Synchronous EL1h
	eret
_do_interrupt_el1_irq:
	call handle_interrupt
	eret
_do_interrupt_fiq_el1h:
	call interrupt_fiq_el1h      // FIQ EL1h
	eret
_do_interrupt_error_el1h:
	call interrupt_error_el1h    // Error EL1h
	eret

_do_interrupt_sync_el0_64:
	call_error interrupt_sync_el0_64   // Synchronous 64-bit EL0
	eret
_do_interrupt_irq_el0_64:
	call interrupt_irq_el0_64    // IRQ 64-bit EL0
	eret
_do_interrupt_fiq_el0_64:
	call interrupt_fiq_el0_64    // FIQ 64-bit EL0
	eret
_do_interrupt_error_el0_64:
	call interrupt_error_el0_64  // Error 64-bit EL0
	eret

_do_interrupt_sync_el0_32:
	call_error interrupt_sync_el0_32   // Synchronous 32-bit EL0
	eret
_do_interrupt_irq_el0_32:
	call interrupt_irq_el0_32    // IRQ 32-bit EL0
	eret
_do_interrupt_fiq_el0_32:
	call interrupt_fiq_el0_32    // FIQ 32-bit EL0
	eret
_do_interrupt_error_el0_32:
	call interrupt_error_el0_32  // Error 32-bit EL0
	eret
	

//	// synchronous
//	.align  7
//	mov     x0, #0
//	mrs     x1, esr_el1
//	mrs     x2, elr_el1
//	mrs     x3, spsr_el1
//	mrs     x4, far_el1
//	b       exception_callback
//
//	// IRQ
//	.align  7
//	mov     x0, #1
//	mrs     x1, esr_el1
//	mrs     x2, elr_el1
//	mrs     x3, spsr_el1
//	mrs     x4, far_el1
//	b       exception_callback
//
//	// FIQ
//	.align  7
//	mov     x0, #2
//	mrs     x1, esr_el1
//	mrs     x2, elr_el1
//	mrs     x3, spsr_el1
//	mrs     x4, far_el1
//	b       exception_callback
//
//	// SError
//	.align  7
//	mov     x0, #3
//	mrs     x1, esr_el1
//	mrs     x2, elr_el1
//	mrs     x3, spsr_el1
//	mrs     x4, far_el1
//	b       exception_callback

install_exception_handlers:
    ldr x0, =_vectors
    msr vbar_el1, x0
	ret