#pragma once

#include <kernel/exceptions.hpp>

namespace exceptions {
	namespace arch {
		namespace arm32 {
			void init();
		}
	}
}

#include "exceptions.inl"
