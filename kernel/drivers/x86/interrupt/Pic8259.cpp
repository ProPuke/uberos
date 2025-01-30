#include "Pic8259.hpp"

#include <kernel/arch/x86/CpuState.hpp>
#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/exceptions.hpp>

namespace driver::interrupt {
	namespace {
		const U16 ioPic1        = 0x20; // master PIC
		const U16 ioPic2        = 0xa0; // slave PIC
		const U16 ioPic1Command = ioPic1;
		const U16 ioPic1Data    = ioPic1+1;
		const U16 ioPic2Command = ioPic2;
		const U16 ioPic2Data    = ioPic2+1;

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

			read_irr = 0x0a,
			read_isr,

			end_of_input = 0x20
		};

		U8 irqOffset[2] = {0,0};
	}

	auto Pic8259::_on_start() -> Try<> {
		irqOffset[0] = 0;
		irqOffset[1] = 0;

		TRY(api.subscribe_ioPort(ioPic1Command));
		TRY(api.subscribe_ioPort(ioPic1Data));
		TRY(api.subscribe_ioPort(ioPic2Command));
		TRY(api.subscribe_ioPort(ioPic2Data));

		min_irq = 0;
		max_irq = 15;

		set_offset(0x20, 0x28); // move away from conflicting interrupts
		disable_all_irqs();

		// enable_irq(0, 1);
		enable_irq(0, 2);
		// enable_irq(0, 12);
		// for(auto i=0;i<16;i++){
		// 	enable_irq(0, i);
		// }

		return {};
	}

	auto Pic8259::_on_stop() -> Try<> {
		return {};
	}

	void Pic8259::enable_irq(U32 cpu, U32 irq) {
		if(cpu>0) return; // seperate/multiple cpus not supported

		arch::x86::IoPort port;
		U8 offset;
		U8 interrupt;

		switch(irq){
			case 0 ... 7:
				port = ioPic1Data;
				offset = irq;
				interrupt = irqOffset[0]+offset;
			break;
			case 8 ... 15:
				port = ioPic2Data;
				offset = irq-8;
				interrupt = irqOffset[1]+offset;
			break;
			default:
				return; //unsupported
		}

		U8 mask = arch::x86::ioPort::read8(port) & ~(1<<offset);
		arch::x86::ioPort::write8(port, mask);

		switch(irq){
			case 0 ... 7:
				log.print_info("enable irq ", irq, " on interrupt ", interrupt);
				api.subscribe_interrupt(interrupt);
			break;
			case 8 ... 15:
				log.print_info("enable irq ", irq, " on interrupt ", interrupt);
				api.subscribe_interrupt(interrupt);
			break;
		}
	}
	void Pic8259::disable_irq(U32 cpu, U32 irq) {
		arch::x86::IoPort port;
		U8 offset;
		U8 interrupt;

		switch(irq){
			case 0 ... 7:
				port = ioPic1Data;
				offset = irq;
				interrupt = irqOffset[0]+irq;
				log.print_info("disable irq ", irq, " on interrupt ", interrupt);
				api.unsubscribe_interrupt(interrupt);
			break;
			case 8 ... 15:
				port = ioPic2Data;
				offset = irq-8;
				interrupt = irqOffset[1]+(irq-8);
				log.print_info("disable irq ", irq, " on interrupt ", interrupt);
				api.unsubscribe_interrupt(interrupt);
			break;
			default:
				return; //unsupported
		}

		auto mask = arch::x86::ioPort::read8(port) | (1<<offset);
		arch::x86::ioPort::write8(port, mask);
	}

	void Pic8259::disable_all_irqs() {
		api.unsubscribe_all_interrupts();

		log.print_info("disable all irqs");

		arch::x86::ioPort::write8(ioPic1Data, 0xff);
		arch::x86::ioPort::write8(ioPic2Data, 0xff);
	}

	void Pic8259::set_offset(U8 offset1, U8 offset2) {
		api.unsubscribe_all_interrupts();

		irqOffset[0] = offset1;
		irqOffset[1] = offset2;

		// get masks
		const auto mask1 = arch::x86::ioPort::read8(ioPic1Data);
		const auto mask2 = arch::x86::ioPort::read8(ioPic2Data);
		
		// icw1
		// begin in cascade mode
		arch::x86::ioPort::write8(ioPic1Command, Command::icw1_init|Command::icw1_icw4);
		arch::x86::ioPort::wait();
		arch::x86::ioPort::write8(ioPic2Command, Command::icw1_init|Command::icw1_icw4);
		arch::x86::ioPort::wait();

		// icw2
		// set vector offsets
		arch::x86::ioPort::write8(ioPic1Data, offset1);
		arch::x86::ioPort::wait();
		arch::x86::ioPort::write8(ioPic2Data, offset2);
		arch::x86::ioPort::wait();

		// icw3
		// tell master it has a slave on 
		arch::x86::ioPort::write8(ioPic1Data, 0b0100); // slave on interrupt 2
		arch::x86::ioPort::wait();
		arch::x86::ioPort::write8(ioPic2Data, 0b0010); // master interrupt 1
		arch::x86::ioPort::wait();
		
		// icw4
		// set to 8086 mode
		arch::x86::ioPort::write8(ioPic1Data, Command::icw4_mode_8086);
		arch::x86::ioPort::wait();
		arch::x86::ioPort::write8(ioPic2Data, Command::icw4_mode_8086);
		arch::x86::ioPort::wait();
		
		// restore masks
		arch::x86::ioPort::write8(ioPic1Data, mask1);
		arch::x86::ioPort::write8(ioPic2Data, mask2);

		for(auto i=0;i<8;i++){
			if(!mask1&(1<<i)) api.subscribe_interrupt(irqOffset[0]+i);
			if(!mask2&(1<<i)) api.subscribe_interrupt(irqOffset[1]+i);
		}
	}

	auto Pic8259::_on_interrupt(U8 vector, const void *_cpuState) -> const void* {
		// const auto &cpuState = *(arch::x86::CpuState*)_cpuState;

		// auto vector = cpuState.interrupt;
		// if(vector>=256) return nullptr;

		if(vector>=irqOffset[0]&&vector<irqOffset[0]+8u){
			const auto irq = vector-irqOffset[0];

			// do not EOI spurious IRQs
			if(irq==7){
				arch::x86::ioPort::write8(ioPic1Command, Command::read_isr);
				if(!arch::x86::ioPort::read8(ioPic1Command)&0b10000000){
					log.print_warning("Spurious IRQ on ", irq);
					return nullptr;
				}
			}

			exceptions::_on_irq(irq);
			arch::x86::ioPort::write8(ioPic1Command, Command::end_of_input);

		}else if(vector>=irqOffset[1]&&vector<irqOffset[1]+8u){
			const auto irq = vector-irqOffset[1]+8;

			// do not EOI spurious IRQs
			if(irq==15){
				arch::x86::ioPort::write8(ioPic2Command, Command::read_isr);
				if(!arch::x86::ioPort::read8(ioPic2Command)&0b10000000){
					log.print_warning("Spurious IRQ on ", irq);
					return nullptr;
				}
			}

			exceptions::_on_irq(irq);
			arch::x86::ioPort::write8(ioPic2Command, Command::end_of_input);
			arch::x86::ioPort::write8(ioPic1Command, Command::end_of_input);
		}

		return nullptr;
	}
}
