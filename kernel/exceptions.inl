#pragma once

#include "exceptions.hpp"

#include <common/types.hpp>
#include <kernel/stdio.hpp>
#include <atomic>

namespace exceptions {
	// extern std::atomic<U32> _lock_depth;
	extern volatile int _lock_depth;

	inline void lock(bool apply) {
		if(__atomic_add_fetch(&_lock_depth, 1, __ATOMIC_SEQ_CST)==0&&apply){
		// if(_lock_depth.fetch_add(1)==0&&apply){
			_deactivate();
		}
	}

	inline void unlock(bool apply) {
		// if(_lock_depth.fetch_sub(1)==1&&apply){
		if(__atomic_sub_fetch(&_lock_depth, 1, __ATOMIC_SEQ_CST)==1&&apply){
			_activate();
		}
	}
}
