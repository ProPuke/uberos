#pragma once

#include <kernel/exceptions.hpp>

namespace exceptions {
	namespace arch {
		namespace arm64 {
			void init();
		}
	}
}

#include "exceptions.inl"
