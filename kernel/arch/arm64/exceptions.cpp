#include "exceptions.hpp"

#include <common/disassemble/arm64.hpp>
#include <kernel/arch/raspi/irq.hpp>
#include <kernel/arch/raspi/mmio.hpp>
#include <kernel/arch/raspi/timer.hpp>
#include <kernel/stdio.hpp>
#include <cstddef>

extern "C" void install_exception_handlers();

namespace timer {
	using namespace timer::arch::raspi;
}

namespace irq {
	using namespace irq::arch::raspi;
}

struct __attribute__((packed)) Registers {
	U64 x[31];
	U64 _;
	U64 elr;
	U64 spsr;
	U64 esr;
	U64 far;
	U64 sctlr;
	U64 tcr;
};

extern "C" Registers exception_error_registers;

Registers exception_error_registers;

namespace exceptions {
	namespace arch {
		namespace arm64 {
			// extern "C" void exception_callback(size_t type, size_t esr, size_t elr, size_t spsr, size_t far) {
			// 	stdio::print("Got exception type ", type, "\n");
			// }

			void handle_error(const char *exception) {
				auto &reg = exception_error_registers;
				const U8 ec = (reg.esr>>26) & 0b111111;

				stdio::print_error_start();
					stdio::print_inline("Error: interrupt ", exception);

					if(exception_error_registers.elr) stdio::print_inline(" from ", (void*)exception_error_registers.elr);
					if(exception_error_registers.far) stdio::print_inline(" touching ", (void*)exception_error_registers.far);
					if(exception_error_registers.esr) stdio::print_inline(" with esr ", (void*)exception_error_registers.esr);
				stdio::print_end();


				const char *error = nullptr;
				switch(ec){
					case 0b000000: error = "Unknown reason (0b000000)"; break;
					case 0b000001: error = "Trapped WF* instruction execution (0b000001)"; break;
					case 0b000011: error = "Trapped MCR or MRC access with (coproc==0b1111) that is not reported using EC 0b000000 (0b000011)"; break;
					case 0b000100: error = "Trapped MCRR or MRRC access with (coproc==0b1111) that is not reported using EC 0b000000 (0b000100)"; break;
					case 0b000101: error = "Trapped MCR or MRC access with (coproc==0b1110) (0b000101)"; break;
					case 0b000110: error = "Trapped LDC or STC access (0b000110)"; break;
					case 0b000111: error = "Access to SVE, Advanced SIMD or floating-point functionality trapped by CPACR_EL1.FPEN, CPTR_EL2.FPEN, CPTR_EL2.TFP, or CPTR_EL3.TFP control (0b000111)"; break;
					case 0b001010: error = "Trapped execution of an LD64B, ST64B, ST64BV, or ST64BV0 instruction (0b001010)"; break;
					case 0b001100: error = "Trapped MRRC access with (coproc==0b1110) (0b001100)"; break;
					case 0b001101: error = "Branch Target Exception (0b001101)"; break;
					case 0b001110: error = "Illegal Execution state (0b001110)"; break;
					case 0b010001: error = "SVC instruction execution in AArch32 state (0b010001)"; break;
					case 0b010101: error = "SVC instruction execution in AArch64 state (0b010101)"; break;
					case 0b011000: error = "Trapped MSR, MRS or System instruction execution in AArch64 state, that is not reported using EC 0b000000, 0b000001, or 0b000111 (0b011000)"; break;
					case 0b011001: error = "Access to SVE functionality trapped as a result of CPACR_EL1.ZEN, CPTR_EL2.ZEN, CPTR_EL2.TZ, or CPTR_EL3.EZ, that is not reported using EC 0b000000 (0b011001)"; break;
					case 0b011100: error = "Exception from a Pointer Authentication instruction authentication failure (0b011100)"; break;
					case 0b100000: error = "Instruction Abort from a lower Exception level (0b100000)"; break;
					case 0b100001: error = "Instruction Abort taken without a change in Exception level (0b100001)"; break;
					case 0b100010: error = "PC alignment fault exception (0b100010)"; break;
					case 0b100100: error = "Data abort (lower level) (0b100100)"; break;
					case 0b100101: error = "Data abort (same level) (0b100101)"; break;
					case 0b100110: error = "SP alignment fault exception (0b100110)"; break;
					case 0b101000: error = "Trapped floating-point exception taken from AArch32 state (0b101000)"; break;
					case 0b101100: error = "Trapped floating-point exception taken from AArch64 state (0b101100)"; break;
					case 0b101111: error = "SError interrupt (0b101111)"; break;
					case 0b110000: error = "Breakpoint exception from a lower Exception level (0b110000)"; break;
					case 0b110001: error = "Breakpoint exception taken without a change in Exception level (0b110001)"; break;
					case 0b110010: error = "Software Step exception from a lower Exception level (0b110010)"; break;
					case 0b110011: error = "Software Step exception taken without a change in Exception level (0b110011)"; break;
					case 0b110100: error = "Watchpoint exception from a lower Exception level (0b110100)"; break;
					case 0b110101: error = "Watchpoint exception taken without a change in Exception level (0b110101)"; break;
					case 0b111000: error = "BKPT instruction execution in AArch32 state (0b111000)"; break;
					case 0b111100: error = "BRK instruction execution in AArch64 state (0b111100)"; break;
				}
			
				if(error){
					stdio::print_error("Error:   ", error);
				}else{
					stdio::print_error("Error:   Unknown (ec = ", ec, ")");
				}

				switch(ec){
					case 0b100100:
					case 0b100101:
						stdio::print_error_start();
							stdio::print_inline("Error:   ");
							switch((reg.esr>>2) & 0b11) {
								case 0b00: stdio::print_inline("Address size fault"); break;
								case 0b01: stdio::print_inline("Translation fault"); break;
								case 0b10: stdio::print_inline("Access flag fault"); break;
								case 0b11: stdio::print_inline("Permission fault"); break;
							}
							
							switch(reg.esr & 0b11) {
								case 0b00: stdio::print_inline(" at level 0"); break;
								case 0b01: stdio::print_inline(" at level 1"); break;
								case 0b10: stdio::print_inline(" at level 2"); break;
								case 0b11: stdio::print_inline(" at level 3"); break;
							}
						stdio::print_end();
					break;
				}

				stdio::print_error("Error:   Registers:");
				stdio::print_error("Error:      0 = ", (void*)reg.x[ 0], "  1 = ", (void*)reg.x[ 1], "  2 = ", (void*)reg.x[ 2], "  3 = ", (void*)reg.x[ 3], "  4 = ", (void*)reg.x[ 4]);
				stdio::print_error("Error:      5 = ", (void*)reg.x[ 5], "  6 = ", (void*)reg.x[ 6], "  7 = ", (void*)reg.x[ 7], "  8 = ", (void*)reg.x[ 8], "  9 = ", (void*)reg.x[ 9]);
				stdio::print_error("Error:     10 = ", (void*)reg.x[10], " 11 = ", (void*)reg.x[11], " 12 = ", (void*)reg.x[12], " 13 = ", (void*)reg.x[13], " 14 = ", (void*)reg.x[14]);
				stdio::print_error("Error:     15 = ", (void*)reg.x[15], " 16 = ", (void*)reg.x[16], " 17 = ", (void*)reg.x[17], " 18 = ", (void*)reg.x[18], " 19 = ", (void*)reg.x[19]);
				stdio::print_error("Error:     20 = ", (void*)reg.x[20], " 21 = ", (void*)reg.x[21], " 22 = ", (void*)reg.x[22], " 23 = ", (void*)reg.x[23], " 24 = ", (void*)reg.x[24]);
				stdio::print_error("Error:     25 = ", (void*)reg.x[25], " 26 = ", (void*)reg.x[26], " 27 = ", (void*)reg.x[27], " 28 = ", (void*)reg.x[28], " 29 = ", (void*)reg.x[29]);
				stdio::print_error("Error:     30 = ", (void*)reg.x[30]);
				stdio::print_error("Error:     elr = ", (void*)reg.elr, "  spsr = ", (void*)reg.spsr , " esr = ", (void*)reg.esr);
				stdio::print_error("Error:     far = ", (void*)reg.far, " sctlr = ", (void*)reg.sctlr, " tcr = ", (void*)reg.tcr);

				if(exception_error_registers.elr){
					if(false){
						stdio::print_error("Error:   Instructions:");
						U32 *current = (U32*)(void*)exception_error_registers.elr;
						U32 *from = (size_t)(void*)current>10*sizeof(U32)?current-10:0;
						U32 *to = (size_t)(void*)current<SIZE_MAX-(1+10)*sizeof(U32)?current+10+1:((U32*)SIZE_MAX)-1;

						for(U32 *i=from; i<current; i++){
							stdio::print_error("Error:     ", (void*)i, " : ", disassemble::arm64::to_string(*i, (U64)(void*)i));
						}
						{
							U32 *i = current;
							stdio::print_error("Error:   > ", (void*)i, " : ", disassemble::arm64::to_string(*current, (U64)(void*)i));
						}
						for(U32 *i=current+1; i<to; i++){
							stdio::print_error("Error:     ", (void*)i, " : ", disassemble::arm64::to_string(*i, (U64)(void*)i));
						}

					}else{
						U32 *current = (U32*)(void*)exception_error_registers.elr;
						U32 *from = (size_t)(void*)current>100*sizeof(U32)?current-100:0;
						U32 *to = (size_t)(void*)current<SIZE_MAX-(1+100)*sizeof(U32)?current+100+1:((U32*)SIZE_MAX)-1;

						stdio::print_error("Error:   Dump from ", to_string_hex_trim((size_t)from), ":");

						stdio::print_error_start();
						stdio::print_inline("Error:     ");
						unsigned x = 0;
						for(U32 *data = from;data<to;data++){
							if(false){
								stdio::print_inline(to_string_hex(*data)+2);
								x += 8;
								if(x>=120){
									stdio::print_end();
									stdio::print_error_start();
									stdio::print_inline("Error:     ");
									x = 0;
								}

							}else{
								stdio::print_inline(to_string_hex(((U8*)data)[0])+2);
								stdio::print_inline(to_string_hex(((U8*)data)[1])+2);
								stdio::print_inline(to_string_hex(((U8*)data)[2])+2);
								stdio::print_inline(to_string_hex(((U8*)data)[3])+2);
								x += 8;
								if(x>=120){
									stdio::print_end();
									stdio::print_error_start();
									stdio::print_inline("Error:     ");
									x = 0;
								}
							}
						}
						stdio::print_end();
					}
				}
			}

			extern "C" void interrupt_sync_el1t() {
				stdio::print_error("");
				handle_error("sync_el1t");
				while(true);
			}
			extern "C" void interrupt_irq_el1t() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt irq_el1t");
				while(true);
			}
			extern "C" void interrupt_fiq_el1t() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt fiq_el1t");
				while(true);
			}
			extern "C" void interrupt_error_el1t() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt error_el1t");
				while(true);
			}
			extern "C" void interrupt_sync_el1h() {
				stdio::print_error("");
				handle_error("sync_el1h");
				while(true);
			}
			extern "C" void interrupt_el1_irq() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt el1_irq");
				while(true);
			}
			extern "C" void interrupt_fiq_el1h() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt fiq_el1h");
				while(true);
			}
			extern "C" void interrupt_error_el1h() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt error_el1h");
				while(true);
			}
			extern "C" void interrupt_sync_el0_64() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt sync_el0_64");
				while(true);
			}
			extern "C" void interrupt_irq_el0_64() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt irq_el0_64");
				while(true);
			}
			extern "C" void interrupt_fiq_el0_64() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt fiq_el0_64");
				while(true);
			}
			extern "C" void interrupt_error_el0_64() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt error_el0_64");
				while(true);
			}
			extern "C" void interrupt_sync_el0_32() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt sync_el0_32");
				while(true);
			}
			extern "C" void interrupt_irq_el0_32() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt irq_el0_32");
				while(true);
			}
			extern "C" void interrupt_fiq_el0_32() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt fiq_el0_32");
				while(true);
			}
			extern "C" void interrupt_error_el0_32() {
				stdio::print_error("");
				stdio::print_error("Error: interrupt error_el0_32");
				while(true);
			}
			
			void init() {
				stdio::Section section("exceptions::arch::arm64::init...");

				irq::init();

				install_exception_handlers();

				exceptions::_activate();
			}
		}
	}
}
