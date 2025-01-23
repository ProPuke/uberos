#pragma once

#include <common/types.hpp>

namespace arch {
	namespace x86 {
		typedef U16 IoPort;

		namespace ioPort {
			auto read8(IoPort) -> U8;
			auto read16(IoPort) -> U16;
			auto read32(IoPort) -> U32;
			void write8(IoPort, U8);
			void write16(IoPort, U16);
			void write32(IoPort, U32);
			void wait();
		}
	}
}

#include "ioPort.inl"
