#pragma once

#include <kernel/drivers/Hardware.hpp>
#include <kernel/arch/x86/PciDevice.hpp>

#include <common/Try.hpp>

namespace driver::system {
	struct Pci final: Hardware {
		DRIVER_INSTANCE(Pci, 0x66d1d64e, "pci", "PCI Bus Controller", Hardware)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto find_device_by_id(U32) -> PciDevice*;
	};
}
