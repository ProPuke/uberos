#include <kernel/CriticalSection.hpp>

namespace mmu {
	auto is_enabled() -> bool;
}

namespace arch {
	namespace arm32 {
		//TODO:allow nested use on the same processor core

		struct Spinlock: NonCopyable<Spinlock> {
			/**/ Spinlock(const char *name):
				name(name)
			{}

			void lock(const char *context = "") {
				CriticalSection::lock();

				#ifdef HAS_SMP
					if(mmu::is_enabled()){
						while (__atomic_test_and_set(&_lock, __ATOMIC_ACQUIRE));
					}
				#endif
			}

			void unlock(bool debug = true) {
				#ifdef HAS_SMP
					if(mmu::is_enabled()||_lock){
						__atomic_clear(&_lock, __ATOMIC_RELEASE);
					}
				#endif

				CriticalSection::unlock();
				// _lock = 0;
			}

			const char *name;

			#ifdef HAS_SMP
				// private:
				volatile U32 _lock = 0;
			#endif
		};
	}
}
