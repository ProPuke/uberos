namespace mmio {
	void barrier() {
		asm volatile(
			"mov r12, #0\n"
			"mcr p15, 0, r12, c7, c10, 5\n"
			"mov pc, lr\n"
		);
	}
}
