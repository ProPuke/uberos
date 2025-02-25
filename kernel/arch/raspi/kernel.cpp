#include <drivers/raspi/serial/Raspi_uart.hpp>

#include <kernel/arch/raspi/atags.hpp>
#include <kernel/arch/raspi/hwquery.hpp>
#include <kernel/arch/raspi/irq.hpp>
#include <kernel/arch/raspi/memory.hpp>
#include <kernel/arch/raspi/serial.hpp>
#include <kernel/arch/raspi/timer.hpp>
#include <kernel/arch/raspi/usb.hpp>
#include <kernel/kernel.hpp>
#include <kernel/logging.hpp>
#include <kernel/mmu.hpp>
#include <kernel/Spinlock.hpp>

#include <common/stdlib.hpp>
#include <common/types.hpp>

namespace arch {
	namespace raspi {
		namespace serial {
			extern driver::serial::Raspi_uart uart0;
		}

		namespace kernel {
			#ifdef HAS_ATAGS
				namespace {
					const ::atags::Atag* atags;
				}
			#endif

			extern "C" [[noreturn]] void entrypoint(const arch::raspi::atags::Atag *_atags) {
				#ifdef HAS_ATAGS
					atags = _atags;
				#endif

				arch::raspi::serial::init();

				::kernel::run();
			}
		}
	}
}

namespace kernel {
	void _preInit() {
		#ifdef HAS_ATAGS
			arch::raspi::atags::init(atags);
		#endif

		arch::raspi::hwquery::init();
		arch::raspi::irq::init();
		arch::raspi::memory::init();

		#ifdef KERNEL_MMU
			mmu::init();
		#endif

		arch::raspi::usb::init();
		arch::raspi::timer::init();

		#ifdef HAS_GIC400
			logging::print_info("interruptController @ ", (void*)&arch::raspi::irq::interruptController);
		#endif
		logging::print_info("cpuInterruptController @ ", (void*)&arch::raspi::irq::cpuInterruptController);
		logging::print_info("uart0 @ ", (void*)&arch::raspi::serial::uart0);

		U64 CurrentEL;
		U64 spsel;
		U64 sp;
		// U64 sp_el0;
		// U64 sp_el1;
		// U64 sp_el2;
		asm volatile("mrs %0, CurrentEL" : "=r" (CurrentEL));
		asm volatile("mrs %0, SPSel" : "=r" (spsel));
		asm volatile("mov %0, sp" : "=r" (sp));
		// asm volatile("mrs %0, sp_el0" : "=r" (sp_el0));
		// asm volatile("mrs %0, sp_el1" : "=r" (sp_el1));
		// asm volatile("mrs %0, sp_el2" : "=r" (sp_el2));

		// asm volatile("msr SPSel, #0\n mov %0, sp" : "=r" (sp_el0));
		// asm volatile("msr SPSel, #1\n mov %0, sp" : "=r" (sp_el1));

		logging::print_info("CurrentEL = ", bits(CurrentEL,2,3));
		logging::print_info("spsel = ", spsel);
		logging::print_info("sp = ", format::Hex64{sp});
		// logging::print_info("sp_el0 = ", format::Hex64{sp_el0});
		// logging::print_info("sp_el1 = ", format::Hex64{sp_el1});
		// logging::print_info("sp_el2 = ", format::Hex64{sp_el2});
	}

	void _postInit(){
		
	}
}
