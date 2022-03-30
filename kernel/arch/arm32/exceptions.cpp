#include "exceptions.hpp"

#include <common/types.hpp>

#include <kernel/stdio.hpp>

#include <atomic>

extern "C" void install_exception_handlers();

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

				install_exception_handlers();

				exceptions::_activate();
			}
		}
	}

	void init() {
		arch::arm32::init();
	}
}
