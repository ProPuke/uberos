#pragma once

#include <kernel/arch/x86/CpuState.hpp>

#include <common/types.hpp>

namespace arch {
	namespace x86 {
		namespace exceptions {
			namespace interrupt {
				typedef const CpuState* (*Subscriber)(const CpuState&);

				void subscribe(U8 vector, Subscriber callback);
				void unsubscribe(U8 vector, Subscriber callback);
				void subscribe_all(Subscriber callback);
				void unsubscribe_all(Subscriber callback);
			}
		}
	}
}
