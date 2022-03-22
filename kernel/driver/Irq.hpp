#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	struct Irq: Driver {
		constexpr /**/ Irq(U64 address, const char *name, const char *descriptiveType):
			Driver(address, name, "irq", descriptiveType)
		{}

		virtual void enable_irq(U32 irq, U8 cpu) = 0;
		virtual void disable_irq(U32 irq) = 0;
	};
}
