#pragma once

#include "exceptions.hpp"

#include <common/types.hpp>
#include <atomic>

namespace exceptions {

	#ifdef ARCH_ARM64
		#ifdef HAS_MMU
			#define HAS_EXCEPTION_ATOMICS
		#endif
	#else
		#define HAS_EXCEPTION_ATOMICS
	#endif

	// extern std::atomic<U32> _lock_depth;
	extern volatile int _lock_depth;

	inline void lock(bool apply) {
		#ifdef HAS_EXCEPTION_ATOMICS
			if(__atomic_add_fetch(&_lock_depth, 1, __ATOMIC_SEQ_CST)==0&&apply){
			// if(_lock_depth.fetch_add(1)==0&&apply){
				_deactivate();
			}
		#else
			if(!_lock_depth++){
				_deactivate();
			}
		#endif
	}

	inline void unlock(bool apply) {
		#ifdef HAS_EXCEPTION_ATOMICS
			// if(_lock_depth.fetch_sub(1)==1&&apply){
			if(__atomic_sub_fetch(&_lock_depth, 1, __ATOMIC_SEQ_CST)==1&&apply){
				_activate();
			}
		#else
			if(!--_lock_depth){
				_activate();
			}
		#endif
	}
}
