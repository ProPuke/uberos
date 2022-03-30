#include <kernel/exceptions.hpp>
#include <kernel/scheduler.hpp>

namespace arch {
	namespace arm32 {
		//TODO:allow nested use on the same processor core

		template <bool lock_scheduler = true, bool lock_exceptions = true>
		struct Spinlock {
			/**/ Spinlock(const char *name):
				name(name)
			{}

			//no accidentally copying
			/**/ Spinlock(const Spinlock&) = delete;
			Spinlock& operator=(const Spinlock&) = delete;

			void lock(const char *context = "") {
				scheduler::lock();
				exceptions::lock();
				while (__atomic_test_and_set(&_lock, __ATOMIC_ACQUIRE));
			}

			void unlock(bool debug = true) {
				__atomic_clear(&_lock, __ATOMIC_RELEASE);
				exceptions::unlock();
				scheduler::unlock();
				// _lock = 0;
			}

			const char *name;

			// private:
			volatile U32 _lock = 0;
		};
	}
}
