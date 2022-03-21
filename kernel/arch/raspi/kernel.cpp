#include "serial.hpp"
#include "memory.hpp"
#include "usb.hpp"
#include "atags.hpp"
#include "timer.hpp"
#include "framebuffer.hpp"
#include "hwquery.hpp"

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
#include <kernel/stdio.hpp>

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
						memory::init();

						#ifdef HAS_MMU
							mmu::init();
						#endif

						framebuffer::arch::raspi::init();
						framebuffer::init();
						usb::init();
						timer::init();
					},

					[] {
						;
					}
				);
			}
		}
	}
}
