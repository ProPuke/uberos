#pragma once

#include <kernel/arch/x86/cpuInfo.hpp>

namespace arch {
	namespace x86 {

		// Model Specific Registers - For P6 CPUs onwards (pentium pro ++)
		namespace msr {
			auto has_msr() -> bool {
				return arch::x86::cpuInfo::get_features().msr;
			}

			void get(U32 msr, U32 &lo, U32 &hi) {
				asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
			}

			void set(U32 msr, U32 lo, U32 hi) {
				asm volatile("wrmsr" :: "a"(lo), "d"(hi), "c"(msr));
			}
		}
	}
}
