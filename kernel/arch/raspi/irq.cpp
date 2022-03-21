#include "irq.hpp"

#include "mmio.hpp"
#include "timer.hpp"

#include <kernel/exceptions.hpp>
#include <kernel/deviceManager.hpp>
#include <kernel/stdio.hpp>
#include <cstddef>

extern "C" void install_exception_handlers();

namespace mmio {
	using namespace arch::raspi;
}

namespace timer {
	using namespace arch::raspi;
}

namespace irq {
	namespace arch {
		namespace raspi {
			struct Interrupt_registers {
				U32 irq_basic_pending; // IRQ_PENDING0
				U32 irq_gpu_pending1;  // IRQ_PENDING1
				U32 irq_gpu_pending2;  // IRQ_PENDING2
				U32 fiq_control;
				U32 irq_gpu_enable1;   // IRQ0_SET_EN_0
				U32 irq_gpu_enable2;   // IRQ0_SET_EN_1
				U32 irq_basic_enable;  // IRQ0_SET_EN_2
				U32 irq_gpu_disable1;  // IRQ0_CLR_EN_0
				U32 irq_gpu_disable2;  // IRQ0_CLR_EN_1
				U32 irq_basic_disable; // IRQ0_CLR_EN_2
			};

			volatile Interrupt_registers &interrupt_registers = *(Interrupt_registers*)mmio::Address::interrupts_pending;

			inline bool _irq_is_pending(U32 irq_basic_pending, U32 irq_gpu_pending2, U32 irq_gpu_pending1, Irq irq) {
				if((U32)irq>=64){
					return irq_basic_pending & (1<<((U32)irq-64));

				}else if((U32)irq>=32){
					return irq_gpu_pending2 & (1<<((U32)irq-32));

				}else{
					return irq_gpu_pending1 & (1<<((U32)irq));
				}
			}

			extern "C" void interrupt_irq() {
				exceptions::Guard guard;
				// stdio::print_debug("irq");

				auto irq_basic_pending = interrupt_registers.irq_basic_pending;
				auto irq_gpu_pending2 = interrupt_registers.irq_gpu_pending2;
				auto irq_gpu_pending1 = interrupt_registers.irq_gpu_pending1;

				for(U32 irq=0;irq<irq_max;irq++){
					if(_irq_is_pending(irq_basic_pending, irq_gpu_pending2, irq_gpu_pending1, (Irq)irq)){
						// stdio::print_debug("irq ", irq);
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
					}
				}
			}

			void init() {
				stdio::Section section("irq::arch::raspi::init...");

				interrupt_registers.irq_basic_disable = 0xffffffff;
				interrupt_registers.irq_gpu_disable1 = 0xffffffff;
				interrupt_registers.irq_gpu_disable2 = 0xffffffff;

				#ifdef HAS_GIC400
					deviceManager::add_device(gic400);
				#endif
			}

			#ifdef HAS_GIC400
				const U32 videocore_peripheral_irqs = 0x60;
			#endif

			void enable(Irq irq) {
				if((U32)irq>=64){
					interrupt_registers.irq_basic_enable = (1<<((U32)irq-64));

				}else if((U32)irq>=32){
					interrupt_registers.irq_gpu_enable2 = (1<<((U32)irq-32));

				}else{
					interrupt_registers.irq_gpu_enable1 = (1<<((U32)irq));
				}

				#ifdef HAS_GIC400
					gic400.enable_irq(videocore_peripheral_irqs+(U32)irq, 0);
				#endif
			}

			void disable(Irq irq) {
				// stdio::print_debug("unset interrupt handler for ", (U32)irq);
				if((U32)irq>=64){
					interrupt_registers.irq_basic_disable = (1<<((U32)irq-64));

				}else if((U32)irq>=32){
					interrupt_registers.irq_gpu_disable2 = (1<<((U32)irq-32));

				}else{
					interrupt_registers.irq_gpu_disable1 = (1<<((U32)irq));
				}

				#ifdef HAS_GIC400
					gic400.disable_irq(videocore_peripheral_irqs+(U32)irq);
				#endif
			}
		}
	}
}
