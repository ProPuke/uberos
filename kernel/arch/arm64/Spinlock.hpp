#include <kernel/exceptions.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/arch/arm64/atomic.hpp>

namespace arch {
	namespace arm64 {
		struct Spinlock {
			/**/ Spinlock(const char *name):
				name(name)
			{}

			//no accidentally copying
			/**/ Spinlock(const Spinlock&) = delete;
			Spinlock& operator=(const Spinlock&) = delete;

			void lock(const char *context) {
				scheduler::lock();
				exceptions::lock();
				// while (__atomic_test_and_set(&_lock, __ATOMIC_ACQUIRE));
				// auto result = arch::arm64::atomic::add_return(&_lock, 69);

				U32 temp;

				asm volatile(
					"sevl\n"
					"0: wfe\n"
					"1: ldaxr %w0, [%1]\n"
					"cbnz %w0, 0b\n"
					"stxr %w0, %w2, [%1]\n"
					"cbnz %w0, 1b\n"
					: "=&r" (temp)
					: "r" (&_lock), "r" (1)
					: "memory"
				);
			}

			void unlock(bool debug = true, bool apply = true) {
				// __atomic_clear(&_lock, __ATOMIC_RELEASE);
				// _lock = 0;

				asm volatile(
					"stlr %w1, [%0]\n"
					:
					: "r" (&_lock), "r" (0)
					: "memory"
				);

				exceptions::unlock();
				scheduler::unlock();
			}

			const char *name;

			// private:
			volatile U32 _lock = 0;
		};
	}
}
