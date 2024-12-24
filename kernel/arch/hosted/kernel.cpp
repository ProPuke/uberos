#include <kernel/arch/x86/cpu.hpp>
#include <kernel/arch/hosted/serial.hpp>

#include <kernel/kernel.hpp>

namespace arch {
	namespace hosted {
		namespace kernel {
			extern "C" void entrypoint() {
				arch::hosted::serial::init();

				::kernel::run();
			}
		}
	}
}

namespace kernel {
	void _preInit() {
		arch::x86::cpu::init();
	}

	void _postInit() {
		
	}
}

int main() {
	::arch::hosted::kernel::entrypoint();
	return 0;
}