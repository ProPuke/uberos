#include "Pic8259.hpp"

#include <kernel/arch/x86/CpuState.hpp>
#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/exceptions.hpp>

namespace driver {
	namespace interrupt {
		namespace {
			const auto pic1        = (arch::x86::IoPort)0x20; // master PIC
			const auto pic2        = (arch::x86::IoPort)0xa0; // slave PIC
			const auto pic1Command = pic1;
			const auto pic1Data    = pic1+1;
			const auto pic2Command = pic2;
			const auto pic2Data    = pic2+1;

			enum Command: U8 {
				// initialisation command words
				icw1_icw4                 = 0x01, // ICW4 will be present
				icw1_mode_single          = 0x02, // single (cascade) mode
				icw1_interval4            = 0x04, // 4 byte interrupt vectors (vs 8)
				icw1_level                = 0x08, // level-triggered mode (vs edge)
				icw1_init                 = 0x10,

				icw4_mode_8086            = 0x01, // 8086/88 mode (vs MCS-80/85)
				icw4_auto_eoi             = 0x02, // auto eoi (vs normal eoi)
				icw4_mode_buffered_slave  = 0x08, // buffered mode slave
				icw4_mode_buffered_master = 0x0C, // buffered mode master
				icw4_mode_sfnm            = 0x10, // special fully nested mode (vs sequential)
			};
		}

		/**/ Pic8259::Pic8259(const char *name):
			Interrupt(name, "interrupt controller"),
			irqOffset{0,0}
		{}

		auto Pic8259::_on_start() -> bool {
			return true;
		}

		auto Pic8259::_on_stop() -> bool {
			return true;
		}

		void Pic8259::enable_irq(U32 cpu, U32 irq) {
			if(cpu>0) return; // seperate/multiple cpus not supported

			arch::x86::IoPort port;
			U8 offset;

			switch(irq){
				case 0 ... 7:
					port = pic1Data;
					offset = irq;
				break;
				case 8 ... 15:
					port = pic2Data;
					offset = irq-8;
				break;
				default:
					return; //unsupported
			}

			auto mask = arch::x86::ioPort::read8(port) & ~(1<<offset);
			arch::x86::ioPort::write8(port, mask);

			switch(irq){
				case 0 ... 7:
					api.subscribe_interrupt(irqOffset[0]+offset);
				break;
				case 8 ... 15:
					api.subscribe_interrupt(irqOffset[1]+offset);
				break;
			}
		}
		void Pic8259::disable_irq(U32 cpu, U32 irq) {
			arch::x86::IoPort port;
			U8 offset;

			switch(irq){
				case 0 ... 7:
					port = pic1Data;
					offset = irq;
					api.unsubscribe_interrupt(irqOffset[0]+irq);
				break;
				case 8 ... 15:
					port = pic2Data;
					offset = irq-8;
					api.unsubscribe_interrupt(irqOffset[1]+(irq-8));
				break;
				default:
					return; //unsupported
			}

			auto mask = arch::x86::ioPort::read8(port) | (1<<offset);
			arch::x86::ioPort::write8(port, mask);
		}

		void Pic8259::disable_all_irqs() {
			api.unsubscribe_all_interrupts();

			arch::x86::ioPort::write8(pic1Data, 0xff);
			arch::x86::ioPort::write8(pic2Data, 0xff);
		}

		void Pic8259::set_offset(U8 offset1, U8 offset2) {
			api.unsubscribe_all_interrupts();

			irqOffset[0] = offset1;
			irqOffset[1] = offset2;

			// get masks
			const auto mask1 = arch::x86::ioPort::read8(pic1Data);
			const auto mask2 = arch::x86::ioPort::read8(pic2Data);
			
			// icw1
			// begin in cascade mode
			arch::x86::ioPort::write8(pic1Command, Command::icw1_init|Command::icw1_icw4);
			arch::x86::ioPort::wait();
			arch::x86::ioPort::write8(pic2Command, Command::icw1_init|Command::icw1_icw4);
			arch::x86::ioPort::wait();

			// icw2
			// set vector offsets
			arch::x86::ioPort::write8(pic1Data, offset1);
			arch::x86::ioPort::wait();
			arch::x86::ioPort::write8(pic2Data, offset2);
			arch::x86::ioPort::wait();

			// icw3
			// tell master it has a slave on 
			arch::x86::ioPort::write8(pic1Data, 0b0100); // slave on interrupt 2
			arch::x86::ioPort::wait();
			arch::x86::ioPort::write8(pic2Data, 0b0010); // master interrupt 1
			arch::x86::ioPort::wait();
			
			// icw4
			// set to 8086 mode
			arch::x86::ioPort::write8(pic1Data, Command::icw4_mode_8086);
			arch::x86::ioPort::wait();
			arch::x86::ioPort::write8(pic2Data, Command::icw4_mode_8086);
			arch::x86::ioPort::wait();
			
			// restore masks
			arch::x86::ioPort::write8(pic1Data, mask1);
			arch::x86::ioPort::write8(pic2Data, mask2);

			for(auto i=0;i<8;i++){
				if(!mask1&(1<<i)) api.subscribe_interrupt(irqOffset[0]+i);
				if(!mask1&(1<<i)) api.subscribe_interrupt(irqOffset[1]+i);
			}
		}

		auto Pic8259::_on_interrupt(void *_cpuState) -> const void* {
			const auto &cpuState = *(arch::x86::CpuState*)_cpuState;

			auto vector = cpuState.interrupt;
			if(vector>=256) return nullptr;

			if(vector>=irqOffset[0]&&vector<irqOffset[0]+8u){
				exceptions::_on_irq(vector-irqOffset[0]);

			}else if(vector>=irqOffset[1]&&vector<irqOffset[1]+8u){
				exceptions::_on_irq(vector-irqOffset[1]);
			}

			return nullptr;
		}
	}
}
