#include "atags.hpp"
#include "framebuffer.hpp"
#include "hwquery.hpp"
#include "irq.hpp"
#include "memory.hpp"
#include "serial.hpp"
#include "timer.hpp"
#include "usb.hpp"

#include <common/stdlib.hpp>
#include <common/types.hpp>

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/exceptions.hpp>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/exceptions.hpp>
#endif

#include <kernel/kernel.hpp>
#include <kernel/mmu.hpp>
#include <kernel/Spinlock.hpp>
#include <kernel/log.hpp>

extern U8 __end;

namespace atags {
	using namespace arch::raspi::atags;
}

namespace serial {
	using namespace arch::raspi::serial;
}

namespace exceptions {
	#if defined(ARCH_ARM32)
		using namespace arch::arm32;
	#elif defined(ARCH_ARM64)
		using namespace arch::arm64;
	#endif
}

namespace irq {
	using namespace arch::raspi;
}

namespace timer {
	using namespace arch::raspi;
}

namespace usb {
	using namespace arch::raspi;
}

namespace memory {
	using namespace arch::raspi;
}

namespace hwquery {
	using namespace arch::raspi;
}

namespace framebuffer {
	using namespace arch::raspi;
}

namespace systemInfo {
	using namespace arch::raspi;
}

#include <kernel/driver/serial/Raspi_uart.hpp>

// namespace serial {
	namespace arch {
		namespace raspi {
			namespace serial {
				extern driver::serial::Raspi_uart uart0;
			}
		}
	}
// }

namespace kernel {
	namespace arch {
		namespace raspi {
			#ifdef HAS_ATAGS
				namespace {
					const ::atags::Atag* atags;
				}
			#endif

			extern "C" void kernel_main(const ::atags::Atag *_atags) {
				#ifdef HAS_ATAGS
					atags = _atags;
				#endif

				kernel::init(
					[] {
						serial::init();
					},

					[] {
						#ifdef HAS_ATAGS
							atags::init(atags);
						#endif

						hwquery::init();
						irq::init();
						memory::init();

						#ifdef HAS_MMU
							mmu::init();
						#endif

						framebuffer::arch::raspi::init();
						framebuffer::init();
						usb::init();
						timer::init();

						log::print_info("interruptController @ ", (void*)&irq::interruptController);
						log::print_info("uart0 @ ", (void*)&::arch::raspi::serial::uart0);

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

						log::print_info("CurrentEL = ", bits(CurrentEL,2,3));
						log::print_info("spsel = ", spsel);
						log::print_info("sp = ", format::Hex64{sp});
						// log::print_info("sp_el0 = ", format::Hex64{sp_el0});
						// log::print_info("sp_el1 = ", format::Hex64{sp_el1});
						// log::print_info("sp_el2 = ", format::Hex64{sp_el2});
					},

					[] {
						;
					}
				);
			}
		}
	}
}
