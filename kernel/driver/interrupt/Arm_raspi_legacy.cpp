#include "Arm_raspi_legacy.hpp"

#include <kernel/mmio.hpp>
#include <kernel/stdio.hpp>

namespace driver {
	namespace interrupt {
		namespace {
			struct Registers {
				U8 _offset1[0x200]; // no idea what goes here..

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
			static_assert(sizeof(Registers)==0x200+40);
		}

		/**/ Arm_raspi_legacy::Arm_raspi_legacy(U32 address):
			Interrupt(address, "Raspi Legacy IRQ System", "interrupt controller")
		{
			max_irq = 72;
		}

		void Arm_raspi_legacy::_on_driver_enable() {
			if(state==State::enabled) return;

			auto &registers = *(volatile Registers*)address;

			registers.irq_basic_disable = 0xffffffff;
			registers.irq_gpu_disable1 = 0xffffffff;
			registers.irq_gpu_disable2 = 0xffffffff;

			state = State::enabled;
		}

		void Arm_raspi_legacy::_on_driver_disable() {
			if(state==State::disabled) return;

			auto &registers = *(volatile Registers*)address;

			registers.irq_basic_disable = 0xffffffff;
			registers.irq_gpu_disable1 = 0xffffffff;
			registers.irq_gpu_disable2 = 0xffffffff;
			
			state = State::disabled;
		}

		void Arm_raspi_legacy::enable_irq(U32 cpu, U32 irq) {
			auto &registers = *(volatile Registers*)address;

			if((U32)irq>=64){
				registers.irq_basic_enable |= (1<<((U32)irq-64));

			}else if((U32)irq>=32){
				registers.irq_gpu_enable2 |= (1<<((U32)irq-32));

			}else{
				registers.irq_gpu_enable1 |= (1<<((U32)irq));
			}
		}

		void Arm_raspi_legacy::disable_irq(U32 cpu, U32 irq) {
			auto &registers = *(volatile Registers*)address;
			
			if((U32)irq>=64){
				registers.irq_basic_disable = (1<<((U32)irq-64));

			}else if((U32)irq>=32){
				registers.irq_gpu_disable2 = (1<<((U32)irq-32));

			}else{
				registers.irq_gpu_disable1 = (1<<((U32)irq));
			}
		}

		namespace {
			inline bool _irq_is_pending(volatile Registers &registers, U32 irq_basic_pending, U32 irq_gpu_pending2, U32 irq_gpu_pending1, U32 irq) {
				if((U32)irq>=64){
					return irq_basic_pending & (1<<((U32)irq-64));

				}else if((U32)irq>=32){
					return irq_gpu_pending2 & (1<<((U32)irq-32));

				}else{
					return irq_gpu_pending1 & (1<<((U32)irq));
				}
			}
		}

		void Arm_raspi_legacy::handle_interrupt(InterruptHandler callback) {
			// exceptions::Guard guard;

			auto &registers = *(volatile Registers*)address;

			auto irq_basic_pending = registers.irq_basic_pending;
			auto irq_gpu_pending2 = registers.irq_gpu_pending2;
			auto irq_gpu_pending1 = registers.irq_gpu_pending1;

			for(U32 irq=0;irq<max_irq;irq++){
				if(_irq_is_pending(registers, irq_basic_pending, irq_gpu_pending2, irq_gpu_pending1, irq)){
					callback(irq);
				}
			}
		}
	}
}
