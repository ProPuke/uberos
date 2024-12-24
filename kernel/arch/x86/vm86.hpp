#pragma once

#include <common/types.hpp>

namespace arch {
	namespace x86 {
		namespace vm86 {
			enum Opcode {
				pushf = 0x9C,
				popf  = 0x9D,
				int_3 = 0xCC,
				_int  = 0xCD,
				iret  = 0xCF,
				cli   = 0xFA,
				sti   = 0xFB,
			};

			// void call_bios(U8 interrupt, IsrRegisters &registers);

			void init();
		}
	}
}
