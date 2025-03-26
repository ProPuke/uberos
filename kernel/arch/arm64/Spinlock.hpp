#include <kernel/arch/arm64/atomic.hpp>
#include <kernel/CriticalSection.hpp>
#include <kernel/mmu.hpp>

namespace mmu {
	auto is_enabled() -> bool;
}

namespace arch {
	namespace arm64 {
		//TODO:allow nested use on the same processor core?

		//NOTE: We might want to handle the case on aarch64 whereby a non-primary core encounters a spinlock without the mmu active. Something has gone very wrong if this happens.
		//If it does, this core should likely just get frozen at this point until the mmu is started and it can correctly enter into the lock.

		struct Spinlock: NonCopyable<Spinlock> {
			/**/ Spinlock(const char *name):
				name(name)
			{}

			void lock(const char *context = "") {
				CriticalSection::lock();

				#ifdef HAS_SMP
					if(mmu::is_enabled()){
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
				#endif
			}

			void unlock(bool debug = true, bool apply = true) {
				#ifdef HAS_SMP
					if(mmu::is_enabled()||_lock){
						// __atomic_clear(&_lock, __ATOMIC_RELEASE);
						// _lock = 0;

						asm volatile(
							"stlr %w1, [%0]\n"
							:
							: "r" (&_lock), "r" (0)
							: "memory"
						);
					}
				#endif

				CriticalSection::unlock();
			}

			const char *name;

			#ifdef HAS_SMP
				// private:
				volatile U32 _lock = 0;
			#endif
		};
	}
}
