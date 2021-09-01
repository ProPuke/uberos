namespace arch {
	namespace raspi {
		namespace mmio {
			inline void barrier() {
				asm volatile("dmb sy");
			}
		}
	}
}
