#pragma once

#include <common/types.hpp>

namespace timer {
	U32 now();
	U64 now64();
	
	void wait(U32 usecs);
	void schedule(U32 usecs, void(*callback)());
	void schedule(U32 usecs, void(*callback)(void *data), void *data);
	void schedule_important(U32 usecs, void(*callback)());
	void schedule_important(U32 usecs, void(*callback)(void *data), void *data);
}


namespace timer {
	inline void schedule(U32 usecs, void(*callback)()) {
		return schedule(usecs, (void(*)(void *data))callback, nullptr);
	}
	inline void schedule_important(U32 usecs, void(*callback)()) {
		return schedule_important(usecs, (void(*)(void *data))callback, nullptr);
	}
}
