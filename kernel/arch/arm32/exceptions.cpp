#include "exceptions.hpp"

#include <kernel/arch/raspi/irq.hpp>
#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/arch/raspi/timer.hpp>
#include <kernel/stdio.hpp>
#include <common/types.hpp>
#include <atomic>

extern "C" void install_exception_handlers();

using namespace arch::raspi;

namespace timer {
	using namespace timer::arch::raspi;
}

namespace irq {
	using namespace irq::arch::raspi;
}

namespace exceptions {
	namespace arch {
		namespace arm32 {
			extern "C" void __attribute__ ((interrupt ("ABORT"))) interrupt_reset() {
				stdio::print_info("RESET HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("ABORT"))) interrupt_prefetch_abort() {
				stdio::print_info("PREFETCH ABORT HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("ABORT"))) interrupt_data_abort() {
				stdio::print_info("DATA ABORT HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("UNDEF"))) interrupt_undefined_instruction() {
				stdio::print_info("UNDEFINED INSTRUCTION HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("SWI"))) interrupt_software_interrupt() {
				stdio::print_info("SWI HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("FIQ"))) interrupt_fast_irq() {
				stdio::print_info("FIQ HANDLER");
				while(true);
			}

			void init() {
				stdio::Section section("exceptions::arch::arm32::init...");

				irq::init();

				install_exception_handlers();

				exceptions::_activate();
			}
		}
	}

	void init() {
		arch::arm32::init();
	}
}
