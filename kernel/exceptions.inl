#pragma once

#include "exceptions.hpp"

#include <common/types.hpp>
#include <atomic>

namespace exceptions {
	#ifdef ARCH_ARM64
		#ifdef HAS_MMU2
			#define HAS_INTERRUPT_ATOMICS
		#endif
	#else
		#define HAS_INTERRUPT_ATOMICS
	#endif

	// extern std::atomic<U32> _lock_depth;
	extern volatile U32 _lock_depth;

	inline void lock(bool apply) {
		#ifdef HAS_INTERRUPT_ATOMICS
			if(__atomic_add_fetch(&_lock_depth, 1, __ATOMIC_SEQ_CST)==1&&apply){
			// if(_lock_depth.fetch_add(1)==0&&apply){
				_deactivate();
			}
		#else
			if(_lock_depth=_lock_depth+1; _lock_depth==1){
				_deactivate();
			}
		#endif
	}

	inline void unlock(bool apply) {
		#ifdef HAS_INTERRUPT_ATOMICS
			// if(_lock_depth.fetch_sub(1)==1&&apply){
			if(__atomic_sub_fetch(&_lock_depth, 1, __ATOMIC_SEQ_CST)==0&&apply){
				_activate();
			}
		#else
			if(_lock_depth=_lock_depth-1; _lock_depth==0){
				_activate();
			}
		#endif
	}
}
