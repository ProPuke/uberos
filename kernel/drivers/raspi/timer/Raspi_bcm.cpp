#include "Raspi_bcm.hpp"

#include <kernel/arch/raspi/irq.hpp>
#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/CriticalSection.hpp>
#include <kernel/log.hpp>

#include <common/types.hpp>

namespace mmio {
	using namespace arch::raspi::mmio;
}

namespace irq {
	using namespace arch::raspi::irq;
}

namespace driver {
	namespace timer {
		namespace {
			//NOTE: do not make this root struct packed as it breaks volatile pointers in gcc (without optimisations) resulting in non-atomic writes
			struct Timer_registers {
				struct __attribute__((packed)) {
					U8 timer_clear: 4;
					U32 reserved: 28;
				} control;

				struct {
					U32 counter_low;
					U32 counter_high;
				};

				U32 timer[4];
			};
			static_assert(sizeof(Timer_registers)==28);

			struct __attribute__((packed)) CoreTimerControlRegisters {
				U8 reserved1;
				bool source:1;
				bool increment:1;
				U32 reserved2:22;
			};
			static_assert(sizeof(CoreTimerControlRegisters)==4);

			// volatile auto &core_timer_control_registers = *(CoreTimerControlRegisters*)mmio::Address::core_timer_control;
		}

		/**/ Raspi_bcm::Raspi_bcm(U64 address, U32 irqAddress, const char *name):
			driver::Timer(address, name, "system timer"),
			irqAddress(irqAddress)
		{
			// log::print_info("timer control: ", mmio::read_address(mmio::Address::core_timer_control));
			// log::print_info("timer prescaler: ", mmio::read_address(mmio::Address::core_timer_prescaler));
		}

		void Raspi_bcm::set_timer(Timer timer, U32 usecs) {
			// log::print_debug("set timer ", (unsigned)timer);

			CriticalSection guard;

			volatile auto &timer_registers = *(Timer_registers*)address;

			usecs += 50;
			// usecs += 5000000;

			{
				mmio::PeripheralAccessGuard guard1;

				timer_registers.control.timer_clear = 1<<(U32)timer;

				//NOTE: SUUPER small values (<4?) might not fire as it takes longer than this to read the time and set the timer
				// we could check current time after to check for this case, but doesn't seem worth it really

				// timer_registers.timer[irqAddress+(U32)timer] = timer_registers.counter_low + usecs;

				for(U32 time; ; usecs+=10){
					U32 low = timer_registers.counter_low;
					timer_registers.timer[(U8)timer] = time = low + usecs;
					U32 lowAfter = timer_registers.counter_low; 
					if(lowAfter>low&&time>lowAfter) break; //make sure this trigger time hasn't been passed already AND that it hasn't been passed, and low hasn't looped around to 0 again so appears before, while actually having passed
				}
			}

			// log::print_debug("set timer ", (U32)timer, ", ", usecs);

			irq::enable((irq::Irq)(irqAddress+(U32)timer));
		}

		U32 Raspi_bcm::now() {
			mmio::PeripheralAccessGuard guard;

			volatile auto &timer_registers = *(Timer_registers*)address;

			return timer_registers.counter_low;
		}

		U64 Raspi_bcm::now64() {
			mmio::PeripheralAccessGuard guard;

			volatile auto &timer_registers = *(Timer_registers*)address;

			// as both high & low timer bits cannot be read at the same time, we'll read high bits first, then low, then update low if high has changed

			U32 firstHigh = timer_registers.counter_high;
			U32 low = timer_registers.counter_low;
			U32 high = timer_registers.counter_high;
			if(high!=firstHigh){
				low = timer_registers.counter_low;
			}

			return ((U64)high)<<32|low;
		}

		__attribute__ ((optimize(0))) void Raspi_bcm::wait(U32 usecs) {
			volatile auto &timer_registers = *(Timer_registers*)address;

			volatile uint32_t curr = timer_registers.counter_low;
			volatile uint32_t x;
			do{
				x = timer_registers.counter_low - curr;
			}while(x<usecs);
		}

		void Raspi_bcm::_on_driver_start() {
			_unsubscribe_all_irqs();
			_subscribe_irq((unsigned)(irqAddress+(U32)Timer::gpu_0));
			_subscribe_irq((unsigned)(irqAddress+(U32)Timer::cpu_0));
			_subscribe_irq((unsigned)(irqAddress+(U32)Timer::gpu_1));
			_subscribe_irq((unsigned)(irqAddress+(U32)Timer::cpu_1));

			Super::_on_driver_start();
		}

		void Raspi_bcm::_on_irq(U32 irq) {
			auto timer = (Timer)irq;

			volatile auto &timer_registers = *(Timer_registers*)address;

			switch(Timer((U32)timer-irqAddress)){
				case Timer::gpu_0:
				case Timer::cpu_0:
				case Timer::gpu_1:
				case Timer::cpu_1:
					{
						mmio::PeripheralAccessGuard guard;
						timer_registers.control.timer_clear = 1<<(U32)timer;
					}
					irq::disable((irq::Irq)timer);

					Raspi_bcm::on_timer(timer);
				break;
			}
		}
	}
}
