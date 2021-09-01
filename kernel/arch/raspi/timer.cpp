#include "timer.hpp"

#include "mmio.hpp"
#include <kernel/arch/arm/scheduler.hpp>
#include "irq.hpp"
#include <common/types.hpp>
#include <kernel/stdio.hpp>

#if defined(ARCH_RASPI1) or defined(ARCH_RASPI2)
	#include "armv7/exceptions.hpp"

	namespace exceptions {
		using namespace exceptions::arch::raspi::armv7;
	}

#elif defined(ARCH_RASPI3) or defined(ARCH_RASPI4)
	#include "armv8/exceptions.hpp"

	namespace exceptions {
		using namespace exceptions::arch::raspi::armv8;
	}
	
#else
	#error "Unsupported architecture"
#endif

namespace irq {
	using namespace irq::arch::raspi;
}

using namespace arch::raspi;

namespace timer {
	namespace arch {
		namespace raspi {
			namespace {
				struct __attribute__((packed)) Timer_registers {
					struct {
						U8 timer_matched: 4;
						U32 reserved: 28;
					} control;

					struct {
						U32 counter_low;
						U32 counter_high;
					};

					U32 timer[4];
				};

				volatile Timer_registers &timer_registers = *(Timer_registers*)mmio::Address::system_timer_base;

				struct __attribute((packed)) CoreTimerControlRegisters {
					U8 reserved1;
					bool source:1;
					bool increment:1;
					U32 reserved2:22;
				};

				volatile CoreTimerControlRegisters core_timer_control_registers = *(CoreTimerControlRegisters*)mmio::Address::core_timer_control;
			}
			
			void init() {
				stdio::Section section("timer::arch::raspi::init...");

				stdio::print_info("timer control: ", mmio::read(mmio::Address::core_timer_control));
				stdio::print_info("timer prescaler: ", mmio::read(mmio::Address::core_timer_prescaler));
			}

			void set_timer(Timer timer, U32 usecs) {
				{
					mmio::PeripheralAccessGuard guard1;

					timer_registers.control.timer_matched = 1<<(U32)timer;

					//NOTE: SUUPER small values (<4?) might not fire as it takes longer than this to read the time and set the timer
					// we could check current time after to check for this case, but doesn't seem worth it really

					// timer_registers.timer[(U8)timer] = timer_registers.counter_low + usecs;

					scheduler::Guard guard2;

					for(U32 time; ; usecs+=10){
						U32 low = timer_registers.counter_low;
						timer_registers.timer[(U8)timer] = time = low + usecs;
						U32 lowAfter = timer_registers.counter_low; 
						if(lowAfter>low&&time>lowAfter) break; //make sure this trigger time hasn't been passed already AND that it hasn't been passed, and low hasn't looped around to 0 again so appears before, while actually having passed
					}
				}

				// stdio::print_error("set timer ", (U32)timer, ", ", usecs);

				switch(timer){
					case Timer::gpu0:
						irq::enable(irq::Irq::system_timer_0);
					break;
					case Timer::cpu_scheduler:
						irq::enable(irq::Irq::system_timer_1);
					break;
					case Timer::gpu1:
						irq::enable(irq::Irq::system_timer_2);
					break;
					case Timer::cpu_slow_scheduler:
						irq::enable(irq::Irq::system_timer_3);
					break;
				}
			}

			void on_interrupt(Timer timer) {
				{
					mmio::PeripheralAccessGuard guard;
					timer_registers.control.timer_matched = 1<<(U32)timer;
				}

				switch(timer){
					case Timer::gpu0:
						irq::disable(irq::Irq::system_timer_0);
					break;
					case Timer::cpu_scheduler:
						// irq::disable(irq::Irq::system_timer_1); //no point? ¯\_(ツ)_/¯
						scheduler::arch::arm::on_timer();
					break;
					case Timer::gpu1:
						irq::disable(irq::Irq::system_timer_2);
					break;
					case Timer::cpu_slow_scheduler:
						// irq::disable(irq::Irq::system_timer_3); //no point? ¯\_(ツ)_/¯
						scheduler::arch::arm::on_slow_timer();
					break;
				}
			}
		}
	}

	using namespace arch::raspi;

	U32 now() {
		mmio::PeripheralAccessGuard guard;

		return timer_registers.counter_low;
	}

	U64 now64() {
		mmio::PeripheralAccessGuard guard;

		// as both high & low timer bits cannot be read at the same time, we'll read high bits first, then low, then update low if high has changed

		U32 firstHigh = timer_registers.counter_high;
		U32 low = timer_registers.counter_low;
		U32 high = timer_registers.counter_high;
		if(high!=firstHigh){
			low = timer_registers.counter_low;
		}

		return ((U64)high)<<32|low;
	}

	__attribute__ ((optimize(0))) void udelay(U32 usecs) {
		volatile uint32_t curr = timer_registers.counter_low;
		volatile uint32_t x;
		do{
			x = timer_registers.counter_low - curr;
		}while(x<usecs);
	}
}
