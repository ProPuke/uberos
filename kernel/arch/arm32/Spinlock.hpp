#include <kernel/exceptions.hpp>

namespace arch {
	namespace arm32 {
		struct Spinlock {
			/**/ Spinlock(const char *name):
				name(name)
			{}

			//no accidentally copying
			/**/ Spinlock(const Spinlock&) = delete;
			Spinlock& operator=(const Spinlock&) = delete;

			void lock(const char *context, bool apply = true) {
				// exceptions::lock(apply);
				while (__atomic_test_and_set(&_lock, __ATOMIC_ACQUIRE));
			}

			void unlock(bool debug = true, bool apply = true) {
				__atomic_clear(&_lock, __ATOMIC_RELEASE);
				// // exceptions::unlock(apply);
				// _lock = 0;
			}

			const char *name;

			// private:
			volatile U32 _lock = 0;
		};
	}
}
