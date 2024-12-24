#pragma once

#include <kernel/drivers/Timer.hpp>

namespace driver {
	namespace timer {
		struct Hpet final: driver::Timer {
			typedef driver::Timer Super;

			/**/ Hpet(const char *name = "HPET", const char *description = "High Precision Event Timer");

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;
		};
	}
}
