#include "irq.hpp"

#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/arch/raspi/timer.hpp>
#include <kernel/CriticalSection.hpp>
#include <kernel/drivers.hpp>
#include <kernel/log.hpp>

namespace irq {
	void on_irq(U32);
}

namespace arch {
	namespace raspi {
		namespace irq {
			#ifdef HAS_GIC400
				driver::interrupt::Arm_gicV2 interruptController {(U32)mmio::Address::gic400};
			#endif

			driver::interrupt::Arm_raspi_legacy cpuInterruptController {(U32)mmio::Address::interrupts_legacy};

			extern "C" void _on_irq() {
				asm volatile("" ::: "memory");

				// U64 sp;
				// asm volatile("mov %0, sp" : "=r" (sp));

				// // I64 distance = sp-(I64)&interruptController.state;
				// I64 distance = sp-(I64)0x00000000000EE0B0;

				// U64 CurrentEL;
				// U64 spsel;
				// asm volatile("mrs %0, CurrentEL" : "=r" (CurrentEL));
				// asm volatile("mrs %0, SPSel" : "=r" (spsel));
				// switch(bits(CurrentEL,2,3)){
				// 	case 0:
				// 		log::print_info("CurrentEL = 0");
				// 	break;
				// 	case 1:
				// 		log::print_info("CurrentEL = 1");
				// 	break;
				// 	case 2:
				// 		log::print_info("CurrentEL = 2");
				// 	break;
				// 	case 3:
				// 		log::print_info("CurrentEL = 3");
				// 	break;
				// 	case 4:
				// 		log::print_info("CurrentEL = 4");
				// 	break;
				// }
				// log::print_info("spsel = ", spsel?"1":"0");

				// log::print_debug(distance>10000?"far1":"near");
				// log::print_debug(distance>1000?"far2":"near");
				// log::print_debug(distance>100?"far3":"near");
				// log::print_debug(distance>10?"far4":"near");
				// log::print_debug(distance<0?"before":"after");
				// log::print_debug(interruptController.state==Driver::State::disabled?"disabled":"?");
				// log::print_debug(interruptController.state==Driver::State::enabled?"enabled":"?");
				// log::print_debug(interruptController.state==Driver::State::restarting?"restarting":"?");
				// log::print_debug(interruptController.state==Driver::State::failed?"failed":"?");
				// log::print_debug((U32)interruptController.state>65536?"high":"low");
				// log::print_debug("irq?");

				// log::print_info("interrupt state = ", interruptController.state);

				if(interruptController.state!=Driver::State::enabled) return;

				// log::print_debug("irq??");

				CriticalSection guard;
				// log::print_debug("irq");


				//FIXME: don't cakk both? only let one delegate irqs?

				#ifdef HAS_GIC400
					interruptController.handle_interrupt(nullptr); //TODO: pass cpu state and handle cpu state return
				#endif

				cpuInterruptController.handle_interrupt(nullptr); //TODO: pass cpu state and handle cpu state return
			}

			void init() {
				log::Section section("arch::raspi::irq::init...");

				#ifdef HAS_GIC400
					drivers::install_driver(interruptController, true);
				#endif
				drivers::install_driver(cpuInterruptController, true);
			}

			void enable(Irq irq) {
				#ifdef HAS_GIC400
					interruptController.enable_irq(0, (U32)irq);
				#endif
				cpuInterruptController.enable_irq(0, (U32)irq);
			}

			void disable(Irq irq) {
				#ifdef HAS_GIC400
					interruptController.disable_irq(0, (U32)irq);
				#endif
				cpuInterruptController.disable_irq(0, (U32)irq);
			}
		}
	}
}
