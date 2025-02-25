#pragma once

#include <drivers/Hardware.hpp>

#include <common/Maybe.hpp>

namespace driver::system {
	struct Smbios final: Hardware {
		DRIVER_INSTANCE(Smbios, 0x73c92949, "smbios", "System Management BIOS", Hardware)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto is_isa_supported() -> Maybe;
		auto is_pci_supported() -> Maybe;
	};
}
