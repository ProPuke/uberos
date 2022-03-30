.section ".text"

.global install_exception_handlers

_vectors:
	ldr pc, interrupt_reset_abs_addr
	ldr pc, interrupt_undefined_instruction_abs_addr
	ldr pc, interrupt_software_interrupt_abs_addr
	ldr pc, interrupt_prefetch_abort_abs_addr
	ldr pc, interrupt_data_abort_abs_addr
	nop                                         // This one is reserved
	ldr pc, interrupt_irq_abs_addr
	ldr pc, interrupt_fast_irq_abs_addr

interrupt_reset_abs_addr:                 .word interrupt_reset
interrupt_undefined_instruction_abs_addr: .word interrupt_undefined_instruction
interrupt_software_interrupt_abs_addr:    .word interrupt_software_interrupt
interrupt_prefetch_abort_abs_addr:        .word interrupt_prefetch_abort
interrupt_data_abort_abs_addr:            .word interrupt_data_abort
interrupt_irq_abs_addr:                   .word interrupt_irq_asm_wrapper
interrupt_fast_irq_abs_addr:              .word interrupt_fast_irq

install_exception_handlers:
	push    {r4-r9}
	ldr     r0, =_vectors
	mov     r1, #0x0000
	ldmia   r0!, {r2-r9}
	stmia   r1!, {r2-r9}
	ldmia   r0!, {r2-r8}
	stmia   r1!, {r2-r8}
	pop     {r4-r9}
	blx     lr

interrupt_irq_asm_wrapper:
	sub     lr, lr, #4       // Adjust return address
	srsdb   sp!, #0x13       // Save irq lr and irq spsp to supervisor stack, and save the resulting stack pointer as the current stack pointer
	cpsid   if, #0x13        // Switch to supervisor mode with interrupts disabled
	push    {r0-r3, r12, lr} // Save the caller save registers
	and     r1, sp, #4       // Make sure stack is 8 byte aligned
	sub     sp, sp, r1
	push    {r1}             // Save the stack adjustment
	bl      handle_interrupt
	pop     {r1}             // Get the stack adjustment
	add     sp, sp, r1
	pop     {r0-r3, r12, lr} // Revert the caller save registers
	rfeia   sp!              // Load the saved return address and program state register from before the interrupt from the stack and return
