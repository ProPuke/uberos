#pragma once

#include "Lock.hpp"

#include <kernel/processor.hpp>

inline void Lock<LockType::flat>::lock() {
	CriticalSection::lock();

	while(true){
		U32 expected = 0;
		if(lockActive.compare_exchange_weak(expected, 1, std::memory_order_acquire)) break;

		do {
			processor::pause();
		}while(lockActive.load(std::memory_order_relaxed) & 1);
	}
}

inline void Lock<LockType::flat>::unlock() {
	lockActive = 0;

	CriticalSection::unlock();
}

inline void Lock<LockType::recursive>::lock() {
	CriticalSection::lock();

	auto processor = processor::get_current_id();

	while(true){
		auto expected = (U32)~0;
		if(lockProcessor.compare_exchange_weak(expected, processor, std::memory_order_acquire) || expected==processor) break;

		do {
			processor::pause();
		}while(lockProcessor.load(std::memory_order_relaxed) != (U32)~0);
	}

	lockDepth++;
}

inline void Lock<LockType::recursive>::unlock() {
	if(lockDepth.fetch_sub(1)==1){
		lockProcessor = (U32)~0;
	}

	CriticalSection::unlock();
}
