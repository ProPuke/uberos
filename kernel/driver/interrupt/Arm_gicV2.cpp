#include "Arm_gicV2.hpp"

#include <kernel/mmio.hpp>
#include <kernel/stdio.hpp>

namespace driver {
	namespace interrupt {
		namespace {
			const U32 gicd_base = 0x1000;

			const U32 vcPeripheralIrqOffset = 96; //videocore interrupts start at SPI ID 96

			struct __attribute__((packed)) Gicd {
				struct __attribute__((packed)) {
					U32 enableGroup0:1;
					U32 enableGroup1:1;
					U32 _reserved1:30;
				} ctlr;

				struct __attribute__((packed)) {
					U32 _interruptCount:5;  // count = 32 * (interruptCount+1)
					U32 cpuCount:3;
					U32 _reserved1:2;
					U32 securityExtensionsImplemented:1;
					U32 lockableSpiCount:5; // only if security extensions implemented
					U32 _reserved2:16;

					U32 get_interrupt_count() volatile {
						return 32*(_interruptCount+1);
					}
				} typer;

				struct __attribute__((packed)) {
					U32 implementer:12; // the JEP106 code of the company that implemented the GIC Distributor
					U32 revision:4;     // usually minor version number
					U32 variant:4;      // usually major version number
					U32 _reserved1:4;
					U32 productId:8;    // product identifier
				} iidr;

				U32 _reserved1[5];
				U32 _reserved2[8];
				U32 _reserved3[16];

				U8 interruptGroup[128];        // each bit marks whether that interrupt is in group 0 or 1 (so 1024 bits total)
				U8 interruptSetEnable[128];    // turn that interrupt on, enabling forwarding to the gcc (or read if it's currently on)
				U8 interruptClearEnable[128];  // turn that interrupt off, disabling forwarding to the gcc (or read if it's currently on, again)

				U8 interruptSetPending[128];   // set each interrupt to pending (or read if it's currently pending)
				U8 interruptClearPending[128]; // set each interrupt to not pending (or read if it's currently pending, again)

				U8 interruptSetActive[128];    // set each interrupt to active (or read if it's currently pending)
				U8 interruptClearActive[128];  // set each interrupt to not active (or read if it's currently pending, again)

				U8  interruptPriority[1020];   // lower values are higher priority (it may use all 8 bits, or may use up to just the last 4, so don't rely on disambiguation <=16

				U32 _reserved4;

				U8 interruptTarget[1020];      // each interrupt, and a bit for each cpu it should go to (bit 0 to cpu 0, bit 1 to cpu1). interrupts 0-31 are readonly

				U32 _reserved5;


				U8 config[256];                // config stuff ¯\_(ツ)_/¯

				U8 _implementationDefined1[256];

				U8 nsacr[256];

				union SoftwareGeneratedInterruptRegister {
					enum struct TargetListFilter {
						cpuTargetList,
						allCpusExceptSender,
						senderOnly
					};

					struct __attribute__((packed)) {
						U8 interruptId:4; // 0..15

						U16 _reserved1:11;

						U8 nsatt:1;       // forward only if the group is set to nsatta (0 or 1) (ignored if security extensions not available)

						U8 cpuTargetList; // a bit for each cpu to forward to (if targetListFilter == cpuTargetList)

						TargetListFilter targetListFilter:2;

						U16 _reserved2:6;
					};

					U32 data;
				};

				U32 _sgir;           // Software Generated Interrupt Register (write-only, using the function below)

				void set_software_generated_interrupt(SoftwareGeneratedInterruptRegister set) volatile {
					_sgir = set.data;
				}

				U32 _reserved6[3];

				U8 softwareClearPending[16];  // set each software interrupt to non-pending. One index per interrupt, one bit per cpu (or read if it's currently pending for that cpu)
				U8 softwareSetPending[16];    // set each software interrupt to pending. One index per interrupt, one bit per cpu (or read if it's currently pending for that cpu)

				U32 _reserved7[40];
				U32 _implementationDefined2[12];
			};
			static_assert(sizeof(Gicd)==4096);

			const U32 gicc_base = 0x2000;

			struct __attribute__((packed)) Gicc {
				struct __attribute__((packed)) {
					U32 enableGroup0:1;
					U32 enableGroup1:1;
					U32 AckCtl:1; //TODO:LABEL
					U32 group0IsFiq:1;
					U32 CBPR:1; //TODO:LABEL
					U32 FIQBypDisGrp0:1; //TODO:LABEL
					U32 IRQBypDisGrp0:1; //TODO:LABEL
					U32 FIQBypDisGrp1:1; //TODO:LABEL
					U32 IRQBypDisGrp1:1; //TODO:LABEL
					U32 EOImodeS:1; //TODO:LABEL
					U32 EOImodeNS:1; //TODO:LABEL
					U32 _reserved1:21;
				} ctlr;

				struct __attribute__((packed)) {
					U8 priority;           // minimum interrupt priority required to forward to the cpu (some lower bits may be ignored, depending how many priority bits are respected)
					U32 _reserved1:24;
				} priorityMask;

				U32 binaryPoint;

				union Iar {
					static const U16 interruptIdNone = 0b1111111111;
					
					struct __attribute__((packed)) {
						U16 interruptId:10;
						U8 softwareCpuSource:3; // if a software interrupt, the cpu that triggered it (otherwise ignore)
						U32 _reserved1:19;
					};

					U32 data;
				} iar;                   // R/O - IAR / interrupt Acknowledge Register. Reading this returns the highest priority interrupt and marks it as acknowledged, or interruptIdNone if none available

				union Eoir {
					struct __attribute__((packed)) {
						U32 interruptId:10;
						U8 softwareCpuSOruce:3; // if a software interrupt, the cpu that triggered it (otherwise ignore)
						U32 _reserved:19;
					};

					U32 data;
				} eoir;                  // W/O - EOIR / End of Interrupt Register. Write exactly back to this what we got from the last IAR, to confirm it
			};
			// static_assert(sizeof(Gicc)==4);

			enum struct Gicc_address: U32 { // cpu interfaces
				ctlr = 0x000, // CPU Interface Control Register
				pmr  = 0x004, // Interrupt Priority Mask Register

				iar  = 0x00c, // Interrupt Acknowledge Register
				eoir = 0x010, // End of Interrupt Register
			};
		}

		void Arm_gicV2::_on_driver_enable() {
			if(state==State::enabled) return;

			auto& gicd = *(volatile Gicd*)(address + gicd_base);
			auto& gicc = *(volatile Gicc*)(address + gicc_base);

			gicd.ctlr.enableGroup0 = 0;
			gicd.ctlr.enableGroup1 = 0;

			gicc.ctlr.enableGroup0 = 0;
			gicc.ctlr.enableGroup1 = 0;
			auto interruptCount = gicd.typer.get_interrupt_count();

			min_irq = 0;
			max_irq = interruptCount;

			for(U32 i=0;i<interruptCount/8;i++){
				gicd.interruptClearEnable[i] = 0xff;
				gicd.interruptClearPending[i] = 0xff;
				gicd.interruptClearActive[i] = 0xff;
			}

			for(U32 i=0;i<16;i++){
				gicd.softwareClearPending[i] = 0xff;
			}

			for(U32 i=0;i<interruptCount/16;i++){
				// mmio::write32(gicd_address+(U32)Gicd_address::irq_config+i*4, 0);
				// gicd.config[i] = 0x0;
			}

			// for(U32 i=0;i<interruptCount;i++){
			// 	mmio::write32(gicd_address+(U32)Gicd_address::icfg+i/16*4, 0b11<<(i%16));
			// }

			stdio::print_info("interruptCount = ", interruptCount);
			stdio::print_info("securityExtensionsImplemented = ", gicd.typer.securityExtensionsImplemented?"yes":"no");

			// for(U32 i=0;i<interruptCount;i++){
			// 	mmio::write32(gicd_address+(U32)Gicd_address::irq_config+4*(i/16), (0b11 << (i%16)) & mmio::read32(gicd_address+(U32)Gicd_address::irq_config+4*(i/16)) | (0b1 << (i%16)));

			// 	mmio::write32(gicd_address+(U32)Gicd_address::irq_setEnable+4*(i/32), 1<<(i%32));
			// 	mmio::write32(gicd_address+(U32)Gicd_address::irq_priority+4*(i), 0xa0);
			// 	mmio::write32(gicd_address+(U32)Gicd_address::irq_target+4*(i), 1<<0);
			// }

			// stdio::print_info("=== 1 ===");

			// for(U32 i=0;i<interruptCount/8;i++){
			// 	stdio::print_info("group ",i);
			// 	gicd.interruptGroup[i] = 0x00;
			// 	// gicd.interruptGroup[i] = 0xff;
			// 	stdio::print_info("enable ",i);
			// 	gicd.interruptSetEnable[i] = 0xff;
			// }

			// stdio::print_info("=== 2 ===");

			// for(U32 i=32;i<interruptCount;i++){
			// 	gicd.interruptTarget[i] = 1<<(0);
			// 	// gicd.interruptPriority[i] = 0xa0;
			// }

			// stdio::print_info("=== 3 ===");

			gicd.ctlr.enableGroup0 = 1;
			gicd.ctlr.enableGroup1 = 1;

			gicc.priorityMask.priority = 0xff;

			gicc.ctlr.enableGroup0 = 1;
			gicc.ctlr.enableGroup1 = 1;

			state = State::enabled;
		}

		void Arm_gicV2::_on_driver_disable() {
			if(state==State::disabled) return;
			
			auto& gicd = *(volatile Gicd*)(address + gicd_base);
			auto& gicc = *(volatile Gicc*)(address + gicc_base);

			gicd.ctlr.enableGroup0 = 0;
			gicd.ctlr.enableGroup1 = 0;

			gicc.ctlr.enableGroup0 = 0;
			gicc.ctlr.enableGroup1 = 0;

			state = State::disabled;
		}

		void Arm_gicV2::enable_irq(U32 cpu, U32 irq) {
			irq += vcPeripheralIrqOffset;

			auto& gicd = *(volatile Gicd*)(address + gicd_base);
			// auto& gicc = *(volatile Gicc*)(address + gicc_base);

			const auto group = 0;

			gicd.interruptGroup[irq/8] = gicd.interruptGroup[irq/8] & ~(1<<(irq%8)) | group<<(irq%8);
			gicd.interruptSetEnable[irq/8] = 1<<(irq%8);
			gicd.interruptPriority[irq] = 0xa0;

			if(irq>=32){
				gicd.interruptTarget[irq] = 1<<cpu; //FIXME:crashes pi 4 on metal
	
			}else if(irq<16){
				Gicd::SoftwareGeneratedInterruptRegister sgir;
				sgir.interruptId = irq;
				sgir.nsatt = 0;
				sgir.cpuTargetList = 1<<cpu;
				sgir.targetListFilter = Gicd::SoftwareGeneratedInterruptRegister::TargetListFilter::cpuTargetList;

				gicd.set_software_generated_interrupt(sgir);
			}
		}

		void Arm_gicV2::disable_irq(U32 cpu, U32 irq) {
			irq += vcPeripheralIrqOffset;

			auto& gicd = *(volatile Gicd*)(address + gicd_base);
			// auto& gicc = *(volatile Gicc*)(address + gicc_base);

			gicd.interruptClearEnable[irq/8] = 1<<(irq%8);

			if(irq>=32){
				gicd.interruptTarget[irq] &= ~(1<<cpu);
			}else if(irq<16){
				; //gets replaced on next set
			}

			if(!gicd.interruptTarget[irq]){
				gicd.interruptClearEnable[irq/8] = 1<<(irq%8);
			}
		}

		// auto Arm_gicV2::get_active_interrupt(U32 cpu) -> U32 {
		// 	// // if(!AnyActiveInterrupts){
		// 	// // 	return 0;
		// 	// // }

		// 	// const U32 gicd_address = address + gicd_base;

		// 	// auto &gicd = *(volatile Gicd*)gicd_address;

		// 	// const auto interruptCount = (void)gicd.typer.interruptCount;
		// 	// const auto enableGroup0 = (void)gicd.ctlr.enableGroup0;
		// 	// const auto enableGroup1 = (void)gicd.ctlr.enableGroup1;

		// 	// U32 hppi = 1023;

		// 	// for(U32 i=0;i<interruptCount;i++){
		// 	// 	const auto groupEnabled = interruptInGroup0(i) && enableGroup0 || interruptInGroup1(i) && enableGroup1;

		// 	// 	const auto isPending = ReadGICD_ITARGETSR(interruptID)==cpu;
		// 	// }

		// 	return 0;
		// }

		void Arm_gicV2::handle_interrupt(InterruptHandler callback) {
			auto& gicc = *(volatile Gicc*)(address + gicc_base);

			Gicc::Iar interrupt;
			interrupt.data = gicc.iar.data; //received?

			if(interrupt.interruptId==Gicc::Iar::interruptIdNone) return; // nothing there ¯\_(ツ)_/¯

			auto irq = interrupt.interruptId - vcPeripheralIrqOffset;

			callback(irq);

			gicc.eoir.data = interrupt.data; //confirmed!
		}
	}
}
