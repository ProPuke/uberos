#include "irq.hpp"

#include "mmio.hpp"
#include "timer.hpp"

#include <kernel/exceptions.hpp>
#include <kernel/stdio.hpp>
#include <cstddef>

extern "C" void install_exception_handlers();

using namespace arch::raspi;

namespace timer {
	using namespace timer::arch::raspi;
}

namespace irq {
	namespace arch {
		namespace raspi {
			struct Interrupt_registers {
				U32 irq_basic_pending;
				U32 irq_gpu_pending1;
				U32 irq_gpu_pending2;
				U32 fiq_control;
				U32 irq_gpu_enable1;
				U32 irq_gpu_enable2;
				U32 irq_basic_enable;
				U32 irq_gpu_disable1;
				U32 irq_gpu_disable2;
				U32 irq_basic_disable;
			};

			volatile Interrupt_registers &interrupt_registers = *(Interrupt_registers*)mmio::Address::interrupts_pending;

			inline bool _irq_is_pending(U32 irq_basic_pending, U32 irq_gpu_pending2, U32 irq_gpu_pending1, Irq irq) {
				if((U32)irq>=64){ //basic
					return irq_basic_pending & (1<<((U32)irq-64));

				}else if((U32)irq>=32){ //gpu2
					return irq_gpu_pending2 & (1<<((U32)irq-32));

				}else{ //gpu1
					return irq_gpu_pending1 & (1<<((U32)irq));
				}
			}

			extern "C" void interrupt_irq() {
				exceptions::Guard guard;
				// stdio::print("IRQ HANDLER ", "\n");
				// int gpu_pending = interrupt_registers.irq_gpu_pending1;

				auto irq_basic_pending = interrupt_registers.irq_basic_pending;
				auto irq_gpu_pending2 = interrupt_registers.irq_gpu_pending2;
				auto irq_gpu_pending1 = interrupt_registers.irq_gpu_pending1;

				bool found = false;

				for(U32 irq=0;irq<irq_max;irq++){
					if(_irq_is_pending(irq_basic_pending, irq_gpu_pending2, irq_gpu_pending1, (Irq)irq)){
						// stdio::print("  FOUND ", irq, "\n");
						found = true;
						// stdio::print("interrupt ", irq, "\n");
						switch((Irq)irq){
							case Irq::system_timer_0:
								// stdio::print("gpu_pending was ",gpu_pending," so firing system_timer_0\n");
								timer::on_interrupt(timer::Timer::gpu0);
								// if(gpu_pending==3){
								// 	stdio::print("gpu_pending is now ",interrupt_registers.irq_gpu_pending1,"\n");
								// }
							break;
							case Irq::system_timer_1:
								// stdio::print("gpu_pending was ",gpu_pending," so firing system_timer_1\n");
								timer::on_interrupt(timer::Timer::cpu_scheduler);
							break;
							case Irq::system_timer_2:
								timer::on_interrupt(timer::Timer::gpu1);
							break;
							case Irq::system_timer_3:
								timer::on_interrupt(timer::Timer::cpu_slow_scheduler);
							break;
							case Irq::usb_controller:
							break;
							case Irq::arm_timer:
							break;
						}
					}
				}
			
				if(!found){
					stdio::print_info("  NOT FOUND");
					stdio::print_info("  irq_basic_pending = ", irq_basic_pending);
					stdio::print_info("  irq_gpu_pending2 = ", irq_gpu_pending2);
					stdio::print_info("  irq_gpu_pending1 = ", irq_gpu_pending1);
				}
			}

			void init() {
				stdio::Section section("irq::arch::raspi::init...");

				interrupt_registers.irq_basic_disable = 0xffffffff;
				interrupt_registers.irq_gpu_disable1 = 0xffffffff;
				interrupt_registers.irq_gpu_disable2 = 0xffffffff;
			}

			void enable(Irq irq) {
				// stdio::print("set interrupt handler for ", (U32)irq, "\n");
				if((U32)irq>=64){ //basic
					interrupt_registers.irq_basic_enable = (1<<((U32)irq-64));

				}else if((U32)irq>=32){ //gpu2
					interrupt_registers.irq_gpu_enable2 = (1<<((U32)irq-32));

				}else{ //gpu1
					interrupt_registers.irq_gpu_enable1 = (1<<((U32)irq));
				}
			}

			void disable(Irq irq) {
				// stdio::print("unset interrupt handler for ", (U32)irq, "\n");
				if((U32)irq>=64){ //basic
					interrupt_registers.irq_basic_disable = (1<<((U32)irq-64));

				}else if((U32)irq>=32){ //gpu2
					interrupt_registers.irq_gpu_disable2 = (1<<((U32)irq-32));

				}else{ //gpu1
					interrupt_registers.irq_gpu_disable1 = (1<<((U32)irq));
				}
			}
		}
	}
}
