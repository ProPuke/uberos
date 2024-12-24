#include "exceptions.hpp"

#include <kernel/exceptions.hpp>
#include <kernel/log.hpp>

#include <common/types.hpp>

#include <atomic>

extern "C" void install_exception_handlers();

namespace arch {
	namespace arm32 {
		namespace exceptions {
			extern "C" void __attribute__ ((interrupt ("ABORT"))) interrupt_reset() {
				log::print_error("RESET HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("ABORT"))) interrupt_prefetch_abort() {
				log::print_error("PREFETCH ABORT HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("ABORT"))) interrupt_data_abort() {
				log::print_error("DATA ABORT HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("UNDEF"))) interrupt_undefined_instruction() {
				log::print_error("UNDEFINED INSTRUCTION HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("SWI"))) interrupt_software_interrupt() {
				log::print_error("SWI HANDLER");
				while(true);
			}
			extern "C" void __attribute__ ((interrupt ("FIQ"))) interrupt_fast_irq() {
				log::print_error("FIQ HANDLER");
				while(true);
			}

			void init() {
				log::Section section("arch::arm32::exceptions::init...");

				install_exception_handlers();

				exceptions::_activate();
			}
		}
	}
}

namespace exceptions {
	void init() {
		arch::arm32::init();
	}
}
