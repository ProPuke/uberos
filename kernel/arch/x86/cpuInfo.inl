#pragma once

#include "cpuInfo.hpp"

namespace arch {
	namespace x86 {
		namespace cpuInfo {
			inline void get_vendor_string(char string[12]) {
				U32 eax, ebx, ecx, edx;
				__cpuid((unsigned)CpuIdRequest::getVendorString, eax, ebx, ecx, edx);

				string[ 0] = ebx>> 0&0xff;
				string[ 1] = ebx>> 8&0xff;
				string[ 2] = ebx>>16&0xff;
				string[ 3] = ebx>>24&0xff;
				string[ 4] = edx>> 0&0xff;
				string[ 5] = edx>> 8&0xff;
				string[ 6] = edx>>16&0xff;
				string[ 7] = edx>>24&0xff;
				string[ 8] = ecx>> 0&0xff;
				string[ 9] = ecx>> 8&0xff;
				string[10] = ecx>>16&0xff;
				string[11] = ecx>>24&0xff;
				string[12] = 0;
			}

			inline auto get_features() -> Features {
				Features features;
				U32 eax, ebx;

				__cpuid((unsigned)CpuIdRequest::getFeatures, eax, ebx, features.ecx, features.edx);

				return features;
			}

			inline void enable_sse() {
				#ifdef _64BIT
					asm volatile(
						"mov rax, cr4\n"
						"or ax, 0x600\n" // enable OSFXSR and OSXMMEXCPT
						"mov cr4, rax\n"
						:
						:
						:
					);
				#else
					asm volatile(
						"mov eax, cr4\n"
						"or ax, 0x600\n" // enable OSFXSR and OSXMMEXCPT
						"mov cr4, eax\n"
						:
						:
						:
					);
				#endif
			}
		}
	}
}
