#pragma once

#include "mmio.hpp"

#include <common/types.hpp>

namespace arch {
	namespace raspi {
		namespace mmio {
			inline void write_address(Address reg, U32 data) {
				return write32((U32)reg, data);
			}
			inline auto read_address(Address reg) -> U32 {
				return read32((U32)reg);
			}
			inline void write_address_offset(U32 offset, Address reg, U32 data) {
				return write32(offset+(U32)reg, data);
			}
			inline auto read_address_offset(U32 offset, Address reg) -> U32 {
				return read32(offset+(U32)reg);
			}
		}
	}
}
