#include "exceptions.hpp"

#include <common/types.hpp>

#include <kernel/log.hpp>

#include <atomic>

extern "C" void install_exception_handlers();

namespace exceptions {
	namespace arch {
		namespace arm32 {
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
				log::Section section("exceptions::arch::arm32::init...");

				install_exception_handlers();

				exceptions::_activate();
			}
		}
	}

	void init() {
		arch::arm32::init();
	}
}
