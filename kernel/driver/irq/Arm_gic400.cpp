#include "Arm_gic400.hpp"

#include <kernel/mmio.hpp>
#include <kernel/stdio.hpp>

namespace driver {
	namespace irq {
		namespace {
			enum struct Gicd_address: U32 { // distributor
				base = 0x1000,

				ctlr  = base,              // Distributor Control Register
				typer = base + 0x004,      // Interrupt Controller Type Register

				irq_group        = base + 0x080, // Interrupt Group Registers
				irq_setEnable    = base + 0x100,
				irq_clearEnable  = base + 0x180,
				irq_setPending   = base + 0x200,
				irq_clearPending = base + 0x280,
				irq_setActive    = base + 0x300,
				irq_clearActive  = base + 0x380,
				irq_priority     = base + 0x400, // Interrupt Priority Registers
				irq_target       = base + 0x800, // Interrupt Processor Targets Registers
				irq_config       = base + 0xc00, // Interrupt Configuration Registers
			};

			enum struct Gicc_address: U32 { // cpu interfaces
				base = 0x2000,

				ctlr = base,         // CPU Interface Control Register
				pmr  = base + 0x004, // Interrupt Priority Mask Register

				iar  = base + 0x0c,  // Interrupt Acknowledge Register
				eoir = base + 0x10 , // End of Interrupt Register
			};	
		}

		void Arm_gic400::enable_driver() {
			if(state==State::enabled) return;

			stdio::Section section("Gic400::init");

			state = State::enabling;

			mmio::write32(address+(U32)Gicd_address::ctlr, 0x00); // disable
			mmio::write32(address+(U32)Gicc_address::ctlr, 0x00); // disable

			U32 interruptCount = (mmio::read32(address+(U32)Gicd_address::typer)&0x1f+1)*32;

			for(U32 i=0;i<interruptCount/32;i++){
				if(!i){
					// enable SGIs (software generated interrupts)
					mmio::write32(address+(U32)Gicd_address::irq_setEnable+i*4, 0x0000ffff);
					mmio::write32(address+(U32)Gicd_address::irq_setEnable+i*4, 0xffff0000);

				}else{
					// disable PPIs and SPIs
					mmio::write32(address+(U32)Gicd_address::irq_clearEnable+i*4, 0xffffffff);
				}
				mmio::write32(address+(U32)Gicd_address::irq_clearPending+i*4, 0xffffffff);
				mmio::write32(address+(U32)Gicd_address::irq_clearActive +i*4, 0xffffffff);
				mmio::write32(address+(U32)Gicd_address::irq_group       +i*4, 0);
			}

			for(U32 i=0;i<interruptCount/4;i++){
				mmio::write32(address+(U32)Gicd_address::irq_priority+i*4, 0x00);
				mmio::write32(address+(U32)Gicd_address::irq_target+i*4, 0x01010101);
			}

			for(U32 i=0;i<interruptCount/16;i++){
				mmio::write32(address+(U32)Gicd_address::irq_config+i*4, 0);
			}

			// for(U32 i=0;i<interruptCount;i++){
			// 	mmio::write32(address+(U32)Gicd_address::icfg+i/16*4, 0b11<<(i%16));
			// }

			stdio::print_info("registerCount = ", interruptCount/32);
			stdio::print_info("interruptCount = ", interruptCount);

			// for(U32 i=0;i<interruptCount;i++){
			// 	mmio::write32(address+(U32)Gicd_address::irq_config+4*(i/16), (0b11 << (i%16)) & mmio::read32(address+(U32)Gicd_address::irq_config+4*(i/16)) | (0b1 << (i%16)));

			// 	mmio::write32(address+(U32)Gicd_address::irq_setEnable+4*(i/32), 1<<(i%32));
			// 	mmio::write32(address+(U32)Gicd_address::irq_priority+4*(i), 0xa0);
			// 	mmio::write32(address+(U32)Gicd_address::irq_target+4*(i), 1<<0);
			// }

			mmio::write32(address+(U32)Gicd_address::ctlr, 0x01); // enable

			mmio::write32(address+(U32)Gicc_address::pmr, 0xf0);
			mmio::write32(address+(U32)Gicc_address::ctlr, 0x01); // enable

			state = State::enabled;
		}

		void Arm_gic400::disable_driver() {
			if(state==State::disabled) return;
			
			state = State::disabling;
			mmio::write32(address+(U32)Gicd_address::ctlr, 0x00);
			mmio::write32(address+(U32)Gicc_address::ctlr, 0x00);
			state = State::disabled;
		}

		void Arm_gic400::enable_irq(U32 irq, U8 cpu) {
			// mmio::write32(address+(U32)Gicd_address::irq_config+4*(irq/16), (0b11 << (irq%16)) & mmio::read32(address+(U32)Gicd_address::irq_config+4*(irq/16)) | (0b1 << (irq%16)));

			// mmio::write32(address+(U32)Gicd_address::irq_setEnable+4*(irq/32), 1<<(irq%32));
			// mmio::write32(address+(U32)Gicd_address::irq_priority+4*(irq), 0xa0);
			// mmio::write32(address+(U32)Gicd_address::irq_target+4*(irq), 1<<cpu);


		}

		void Arm_gic400::disable_irq(U32 irq) {

		}

		// intel GIC (1?) docs:

		// void Arm_gic400::init() {
		// 	stdio::Section section("Arm_gic400::init");

		// 	// Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all
		// 	// priorities
		// 	*((int *) 0xFFFEC104) = 0xFFFF;
		// 	// Set CPU Interface Control Register (ICCICR). Enable signaling of
		// 	// interrupts
		// 	*((int *) 0xFFFEC100) = 1;
		// 	mmio::write32(address+(U32)Gicc_address::ctlr, 1);
		// 	// Configure the Distributor Control Register (ICDDCR) to send pending
		// 	// interrupts to CPUs
		// 	*((int *) 0xFFFED000) = 1;
		// 	mmio::write32(address+(U32)Gicd_address::ctlr, 1);
		// }

		// void Arm_gic400::enable_irq(U32 irq, U8 cpu) {
		// 	int reg_offset, index, value, address;

		// 	/* Configure the Interrupt Set-Enable Registers (ICDISERn).
		// 	* reg_offset = (integer_div(N / 32) * 4
		// 	* value = 1 << (N mod 32) */
		// 	reg_offset = (irq >> 3) & 0xFFFFFFFC;
		// 	index = irq & 0x1F;
		// 	value = 0x1 << index;

		// 	/* Now that we know the register address and value, set the appropriate bit */
		// 	mmio::write32(address+(U32)Gicd_address::irq_setEnable + reg_offset, mmio::read32(address+(U32)Gicd_address::irq_setEnable + reg_offset) | value);

		// 	/* Configure the Interrupt Processor Targets Register (ICDIPTRn)
		// 	* reg_offset = integer_div(N / 4) * 4
		// 	* index = N mod 4 */
		// 	reg_offset = (irq & 0xFFFFFFFC);
		// 	index = irq & 0x3;
		// 	/* Now that we know the register address and value, write to (only) the
		// 	* appropriate byte */
		// 	mmio::write32(address+(U32)Gicd_address::irq_target + reg_offset + index, cpu);
		// }
	}
}
