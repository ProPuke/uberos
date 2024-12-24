#pragma once

#include <common/types.hpp>

namespace arch {
	namespace x86 {
		namespace idt {
			void set_entry(U8 i, void *isr, U8 flags);
			void apply_entries();
		}
	}
}
