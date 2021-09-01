#include "serial.hpp"
#include "memory.hpp"
#include "usb.hpp"
#include "atags.hpp"
#include "timer.hpp"
#include "framebuffer.hpp"
#include "../arm/kernel.hpp"

#if defined(ARCH_RASPI1) or defined(ARCH_RASPI2)
	#include "armv7/exceptions.hpp"
#elif defined(ARCH_RASPI3) or defined(ARCH_RASPI4)
	#include "armv8/exceptions.hpp"
#endif

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
	#if defined(ARCH_RASPI1) or defined(ARCH_RASPI2)
		using namespace exceptions::arch::raspi::armv7;
	#elif defined(ARCH_RASPI3) or defined(ARCH_RASPI4)
		using namespace exceptions::arch::raspi::armv8;
	#endif
}

namespace timer {
	using namespace arch::raspi;
}

namespace usb {
	using namespace arch::raspi::usb;
}

namespace memory {
	using namespace arch::raspi;
}

namespace framebuffer {
	using namespace arch::raspi;
}

namespace kernel {
	namespace arch {
		namespace arm {
			namespace raspi {
				extern "C" void kernel_main(size_t _atags) {
					auto atags = (atags::Atag*) _atags;

					serial::init();

					{
						stdio::Section section("kernel::arch::raspi startup");

						atags::init(atags);
						memory::init();
						libc::init();

						framebuffer::init();

						usb::init();
						exceptions::init();
						timer::init();
					}

					kernel::arch::arm::init();
				}
			}
		}
	}
}
