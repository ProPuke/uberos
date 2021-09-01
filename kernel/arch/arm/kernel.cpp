#include "kernel.hpp"

#include "scheduler.hpp"
#include <kernel/stdio.hpp>
#include <kernel/memory.hpp>
#include <kernel/kernel.hpp>

namespace kernel {
	namespace arch {
		namespace arm {
			void init() {
				{
					stdio::Section section("kernel::arch::arm startup");

					scheduler::arch::arm::init();
				}

				::kernel::init();
			}
		}
	}
}
