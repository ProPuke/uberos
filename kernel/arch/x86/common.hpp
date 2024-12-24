#pragma once

#include <common/types.hpp>

namespace arch {
	namespace x86 {
		struct Registers {
			union {
				U64 rax;
				U32 eax;
				U16 ax;
				struct {
					U8 ah;
					U8 al;
				};
			};
			union {
				U64 rbx;
				U32 rbx;
				U16 bx;
				struct {
					U8 bh;
					U8 bl;
				};
			};
			union {
				U64 rcx;
				U32 rcx;
				U16 cx;
				struct {
					U8 ch;
					U8 cl;
				};
			};
			union {
				U64 rdx;
				U32 rdx;
				U16 dx;
				struct {
					U8 dh;
					U8 dl;
				};
			};
		};

		inline auto interrupt(U16 interrupt, Registers registers) -> Registers;
	}
}
