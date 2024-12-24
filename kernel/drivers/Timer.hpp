#pragma once

#include <kernel/console.hpp>
#include <kernel/Driver.hpp>

#include <functional>

namespace driver {
	struct Timer: Driver {
		typedef Driver Super;

		static DriverType driverType;

		/**/ Timer(const char *name, const char *description);

		virtual U32 now() = 0;
		virtual U64 now64() = 0;
		virtual void wait(U32 usecs) = 0;
	};
}
