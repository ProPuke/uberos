#pragma once

#include <kernel/drivers/Hardware.hpp>

#include <common/Maybe.hpp>

namespace driver::system {
	struct Smbios final: Hardware {
		DRIVER_INSTANCE(Smbios, "smbios", "System Management BIOS", Hardware)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto is_isa_supported() -> Maybe;
		auto is_pci_supported() -> Maybe;
	};
}
