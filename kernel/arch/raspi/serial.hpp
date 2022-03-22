#pragma once

#if !defined(ARCH_RASPI_UART0) && !defined(ARCH_RASPI_UART1)
	#define ARCH_RASPI_UART1
#endif

namespace arch {
	namespace raspi {
		namespace serial {
			void init();
		}
	}
}
