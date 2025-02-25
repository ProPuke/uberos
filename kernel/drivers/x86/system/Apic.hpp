#pragma once

#include <kernel/drivers/Hardware.hpp>

#include <common/Try.hpp>

namespace driver::system {
	//TODO: merge into a 8259 PIC driver?

	struct Apic final: Hardware {
		DRIVER_INSTANCE(Apic, 0x5abbdcf2, "apic", "Advanced Programmable Interrupt Controller", Hardware)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;
	};
}
