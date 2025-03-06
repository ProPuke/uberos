#pragma once

namespace arch {
	namespace x86 {
		namespace nmi {
			void enable();
			void disable();

			auto isEnabled() -> bool;
		}
	}
}
