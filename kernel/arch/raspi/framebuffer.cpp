#include <kernel/drivers/raspi/graphics/Raspi_videocore_mailbox.hpp>
#include <kernel/arch/raspi/mmio.hpp>

namespace arch {
	namespace raspi {
		namespace framebuffer {
			driver::graphics::Raspi_videocore_mailbox device((U64)arch::raspi::mmio::Address::mail0_base);
		}
	}
}
