#include "scheduler.hpp"

#include <kernel/Thread.hpp>
#include <kernel/ThreadCpuState.hpp>
#include <kernel/stdio.hpp>
#include <kernel/memory.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/mmu.hpp>
#include <kernel/kernel.h>
#include <atomic>

#if defined(ARCH_RASPI)
	#include "../raspi/timer.hpp"
#else
	#error "Unsupported architecture"
#endif

namespace timer {
	using namespace timer::arch::raspi;
}

namespace thread {
	extern LList<Thread> activeThreads;
	extern LList<Thread> sleepingThreads;
	extern LList<Thread> pausedThreads;
	extern LList<Thread> freedThreads;
}

extern U8 __end;

namespace scheduler {
	namespace arch {
		namespace arm {
			std::atomic<unsigned> lock_depth = 0;

			U32 interval = 15000; //15ms
			U32 slowInterval = interval*3;

			U64 lastSchedule = 0;
			U64 lastPing = 0;
			U32 deferredYields = 0;
			
			Spinlock threadLock("scheduler threadLock");

			void init() {
				stdio::Section section("scheduler::arch::arm::init...");

				stdio::print_debug("installing interrupt timer...");
				// {
				// 	stdio::print_debug("creating test spinlock");
				// 	Spinlock testlock("testlock");
				// 	Spinlock *testlock2 = (Spinlock*)0xdf000;
				// 	stdio::print_debug("spinlock @ ", &testlock);
				// 	stdio::print_debug("spinlock2 @ ", testlock2);
				// 	testlock2->_lock = 0;
				// 	stdio::print_debug("testing test spinlock 2");
				// 	{
				// 		Spinlock_Guard _guard(*testlock2, "testlock tester", true);

				// 		stdio::print_debug("mid test 2");
				// 	}
				// 	stdio::print_debug("testing test spinlock 1");
				// 	{
				// 		Spinlock_Guard _guard(testlock, "testlock tester", true);

				// 		stdio::print_debug("mid test 1");
				// 	}
				// 	stdio::print_debug("tested test spinlock");
				// }
				timer::set_timer(timer::Timer::cpu_scheduler, 1);
				timer::set_timer(timer::Timer::cpu_scheduler, 1);
				//8436924000
				//8440321000

				// stdio::print_info("test kmalloc");
				// memory::kmalloc(123);

				stdio::print_info("allocating main process...");
				auto mainProcess = new Process("kernel");
				auto mainThread = mainProcess->create_current_thread(memory::get_memory_page(&__end), KERNEL_STACK_SIZE);

				// thread::activeThreads.push_back(*mainThread);
				::thread::currentThread = mainThread;

				#if defined(ARCH_RASPI)
					stdio::print_info("managed by timer::arch::raspi");
					timer::set_timer(timer::Timer::cpu_scheduler, interval);
					timer::set_timer(timer::Timer::cpu_slow_scheduler, interval);

				#else
					#error "Unsupported architecture"
				#endif
			}

			void on_timer() {
				yield();
			}

			std::atomic<U32> scheduledTime = 0;

			void on_slow_timer() {
				// threadLock.lock("on_slower_timer() threadLock");

				// // stdio::print("ping ", now, "\n");


				// threadLock.unlock(false);

				timer::set_timer(timer::Timer::cpu_slow_scheduler, slowInterval);
			}
		}
	}

	using namespace arch::arm;

	void init() {
		arch::arm::init();
	}

	void yield() {
		if(lock_depth>0){
			deferredYields++;
			return;
		}

		// bool exceptionsWereActive = exceptions::_is_active();

		lock();
		exceptions::lock();

		auto now = timer::now();

		// if(now-lastSchedule<interval*3/4){
		// 	stdio::print_info("fast: ", now-lastSchedule);
		// }else if(now-lastSchedule>interval*6/5){
		// 	stdio::print_info("slow: ", now-lastSchedule);
		// }
		lastSchedule = now;

		threadLock.lock("yield() threadlock");

		// wake sleeping threads
		for(Thread *thread = thread::sleepingThreads.head; thread && now >= thread->sleep_wake_time && now-thread->sleep_wake_time < 1ull<<63; thread = thread::sleepingThreads.head) {
			thread->state = Thread::State::active;
			thread::sleepingThreads.pop_front();
			thread::activeThreads.push_front(*thread);
		}

		auto oldThread = ::thread::currentThread.load();

		if(thread::activeThreads.size==0||thread::activeThreads.size==1&&thread::activeThreads.head==oldThread){
			//if no other threads to switch to, we can ease up on the scheduling a bit..

			threadLock.unlock(false);
			timer::set_timer(timer::Timer::cpu_scheduler, interval*4);
			unlock();
			exceptions::unlock();

		}else{
			// if the current thread has been scheduled again, then put it at back of the queue and we're done. Nothing to swap
			if(thread::activeThreads.head==oldThread){
				thread::activeThreads.push_back(*thread::activeThreads.pop_front());

				threadLock.unlock(false);
				timer::set_timer(timer::Timer::cpu_scheduler, interval);
				unlock();
				exceptions::unlock();

				return;
			}

			auto &newThread = *thread::activeThreads.pop_front();

			if(newThread.state==Thread::State::active){
				thread::activeThreads.push_back(newThread);
			}

			::thread::currentThread = &newThread;

			threadLock.unlock(false);
			// exceptions::unlock(false);
			// exceptions::_activate();
			scheduledTime = timer::now();
			timer::set_timer(timer::Timer::cpu_scheduler, interval);

			deferredYields = 0; //nothing was missed as we've just left, do not auto fire any on unlock()
			unlock();

			asm volatile("" ::: "memory");

			#ifdef HAS_MMU
				if(newThread.process.memoryMapping.pageCount>0){
					mmu::set_userspace_mapping(newThread.process.memoryMapping);
				}else{
					mmu::set_userspace_mapping(mmu::kernelMapping);
				}
			#endif

			// if(oldThread){
			// 	stdio::print_debug("jump from ", (void*)oldThread, " pc = ", (void*)oldThread->storedState->pc, " lr = ", (void*)oldThread->storedState->lr);
			// }
			// stdio::print_debug("jump to   ", (void*)&newThread, " pc = ", (void*)newThread.storedState->pc, " lr = ", (void*)newThread.storedState->lr);

			Thread::swap_state(*oldThread, newThread);

			// stdio::print_debug("swapped");

			exceptions::unlock();
		}
	}

	void lock() {
		lock_depth.fetch_add(1);
	}

	void unlock() {
		if(lock_depth.fetch_sub(1)==1){
			if(deferredYields>0){
				deferredYields = 0;
				yield();
			}
		}
	}

	U32 get_total_thread_count() {
		Spinlock_Guard lock(threadLock, "get_total_thread_count");
		return thread::activeThreads.size + thread::sleepingThreads.size + thread::pausedThreads.size;
	}

	U32 get_active_thread_count() {
		Spinlock_Guard lock(threadLock, "get_active_thread_count");
		return thread::activeThreads.size;
	}

}
