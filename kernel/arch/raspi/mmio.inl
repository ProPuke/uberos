#pragma once

#include "mmio.hpp"

#include <common/types.hpp>

namespace mmio {
	namespace arch {
		namespace raspi {
			inline void write_address(Address reg, U32 data) {
				return mmio::write32((U32)reg, data);
			}
			inline U32 read_address(Address reg) {
				return mmio::read32((U32)reg);
			}
		}
	}
}
