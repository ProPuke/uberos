#pragma once

#include "Lock.hpp"

#include <kernel/processor.hpp>

inline void Lock<LockType::flat>::lock() {
	CriticalSection::lock();

	#ifdef HAS_SMP
		while(true){
			U32 expected = 0;
			if(lockActive.compare_exchange_weak(expected, 1, std::memory_order_acquire)) break;

			do {
				processor::pause();
			}while(lockActive.load(std::memory_order_relaxed) & 1);
		}
	#endif
}

inline void Lock<LockType::flat>::unlock() {
	#ifdef HAS_SMP
		lockActive = 0;
	#endif

	CriticalSection::unlock();
}

inline void Lock<LockType::recursive>::lock() {
	CriticalSection::lock();

	#ifdef HAS_SMP
		auto processor = processor::get_active_id();

		while(true){
			auto expected = (U32)~0;
			if(lockProcessor.compare_exchange_weak(expected, processor, std::memory_order_acquire) || expected==processor) break;

			do {
				processor::pause();
			}while(lockProcessor.load(std::memory_order_relaxed) != (U32)~0);
		}

		lockDepth++;
	#endif
}

inline void Lock<LockType::recursive>::unlock() {
	#ifdef HAS_SMP
		if(lockDepth.fetch_sub(1)==1){
			lockProcessor = (U32)~0;
		}
	#endif

	CriticalSection::unlock();
}
