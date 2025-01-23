#include <kernel/CriticalSection.hpp>

namespace arch {
	namespace x86 {
		//TODO:allow nested use on the same processor core?

		struct Spinlock {
			/**/ Spinlock(const char *name):
				name(name)
			{}

			//no accidentally copying
			/**/ Spinlock(const Spinlock&) = delete;
			Spinlock& operator=(const Spinlock&) = delete;

			void lock(const char *context = "") {
				CriticalSection::lock();

				asm volatile(
					"acquire%=:\n"
					"  lock bts dword ptr [%0], 0\n"
					"  jnc done%=\n"
					"waitForRetry%=:\n"
					"  pause\n"
					"  test dword ptr [%0], 1\n"
					"  jne waitForRetry%=\n"
					"  jmp acquire%=\n"
					"done%=:\n"
					:
					: "r" (&_lock)
				);
			}

			void unlock(bool debug = true, bool apply = true) {
				asm volatile(
					"mov dword ptr [%0], 0"
					:
					: "r" (&_lock)
				);

				CriticalSection::unlock();
			}

			const char *name;

			private:

			volatile U32 _lock = 0;
		};
	}
}
