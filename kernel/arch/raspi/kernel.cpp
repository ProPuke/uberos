#include "serial.hpp"
#include "memory.hpp"
#include "usb.hpp"
#include "atags.hpp"
#include "timer.hpp"
#include "framebuffer.hpp"
#include "hwquery.hpp"
#include "../arm/kernel.hpp"

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/exceptions.hpp>
	#include <kernel/arch/arm32/mmu.hpp>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/exceptions.hpp>
	#include <kernel/arch/arm64/mmu.hpp>
#endif

#include <kernel/Spinlock.hpp>

#include <kernel/libc.hpp>
#include <kernel/stdio.hpp>
#include <common/stdlib.hpp>
#include <common/types.hpp>

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

namespace mmu {
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
		namespace arm {
			namespace raspi {
				extern "C" void kernel_main(size_t _atags) {
					// auto atags = (atags::Atag*) _atags;

					serial::init();

					{
						stdio::Section section("kernel::arch::raspi startup");

						libc::init();
						// atags::init(atags);
						exceptions::init();
						hwquery::init();
						memory::init();
						mmu::init();
						framebuffer::init();
						usb::init();
						timer::init();
					}

					kernel::arch::arm::init();
				}
			}
		}
	}
}
