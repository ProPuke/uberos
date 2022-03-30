#pragma once

#include <kernel/scheduler.hpp>
#include <kernel/Spinlock.hpp>
#include <common/types.hpp>

namespace scheduler {
	namespace arch {
		namespace arm {
			extern Spinlock<> threadLock; //TODO:remove

			void init();

			void on_timer();
			void on_slow_timer();
		}
	}
}
