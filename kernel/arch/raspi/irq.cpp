#include "irq.hpp"

#include "mmio.hpp"
#include "timer.hpp"

#include <kernel/CriticalSection.hpp>
#include <kernel/device.hpp>
#include <kernel/log.hpp>

namespace mmio {
	using namespace arch::raspi;
}

namespace timer {
	using namespace arch::raspi;
}

namespace irq {
	namespace arch {
		namespace raspi {
			#ifdef HAS_GIC400
				driver::interrupt::Arm_gicV2 interruptController {(U32)mmio::Address::gic400};
			#endif

			driver::interrupt::Arm_raspi_legacy cpuInterruptController {(U32)mmio::Address::interrupts_legacy};

			extern "C" void handle_interrupt() {
				asm volatile("" ::: "memory");

				U64 sp;
				asm volatile("mov %0, sp" : "=r" (sp));

				// I64 distance = sp-(I64)&interruptController.state;
				I64 distance = sp-(I64)0x00000000000EE0B0;

				U64 CurrentEL;
				U64 spsel;
				asm volatile("mrs %0, CurrentEL" : "=r" (CurrentEL));
				asm volatile("mrs %0, SPSel" : "=r" (spsel));
				switch(bits(CurrentEL,2,3)){
					case 0:
						log::print_info("CurrentEL = 0");
					break;
					case 1:
						log::print_info("CurrentEL = 1");
					break;
					case 2:
						log::print_info("CurrentEL = 2");
					break;
					case 3:
						log::print_info("CurrentEL = 3");
					break;
					case 4:
						log::print_info("CurrentEL = 4");
					break;
				}
				log::print_info("spsel = ", spsel?"1":"0");

				log::print_debug(distance>10000?"far1":"near");
				log::print_debug(distance>1000?"far2":"near");
				log::print_debug(distance>100?"far3":"near");
				log::print_debug(distance>10?"far4":"near");
				log::print_debug(distance<0?"before":"after");
				log::print_debug(interruptController.state==Driver::State::disabled?"disabled":"?");
				log::print_debug(interruptController.state==Driver::State::enabled?"enabled":"?");
				log::print_debug(interruptController.state==Driver::State::restarting?"restarting":"?");
				log::print_debug(interruptController.state==Driver::State::failed?"failed":"?");
				log::print_debug((U32)interruptController.state>65536?"high":"low");
				log::print_debug("irq?");

				// log::print_info("interrupt state = ", interruptController.state);

				if(interruptController.state!=Driver::State::enabled) return;

				log::print_debug("irq??");

				CriticalSection guard;
				log::print_debug("irq");

				interruptController.handle_interrupt([](U32 irq) {
					log::print_debug("irq ", irq);

					switch((Irq)irq){
						case Irq::system_timer_0:
							timer::on_interrupt(timer::Timer::gpu0);
						break;
						case Irq::system_timer_1:
							timer::on_interrupt(timer::Timer::cpu_scheduler);
						break;
						case Irq::system_timer_2:
							timer::on_interrupt(timer::Timer::gpu1);
						break;
						case Irq::system_timer_3:
							timer::on_interrupt(timer::Timer::cpu_slow_scheduler);
						break;
						// case 0x60:
						// 	timer::on_interrupt(timer::Timer::gpu0);
						// break;
						// case 0x61:
						// 	timer::on_interrupt(timer::Timer::cpu_scheduler);
						// break;
						// case 0x62:
						// 	timer::on_interrupt(timer::Timer::gpu1);
						// break;
						// case 0x63:
						// 	timer::on_interrupt(timer::Timer::cpu_slow_scheduler);
						// break;
						case Irq::usb_controller:
						break;
						case Irq::hdmi_0:
						case Irq::hdmi_1:
						break;
						case Irq::arm_timer:
						break;
					}
				});
			}

			void init() {
				log::Section section("irq::arch::raspi::init...");

				device::install_device(interruptController, true);
				device::install_device(cpuInterruptController, true);
			}

			void enable(Irq irq) {
				interruptController.enable_irq(0, (U32)irq);
				cpuInterruptController.enable_irq(0, (U32)irq);
			}

			void disable(Irq irq) {
				interruptController.disable_irq(0, (U32)irq);
				cpuInterruptController.disable_irq(0, (U32)irq);
			}
		}
	}
}
