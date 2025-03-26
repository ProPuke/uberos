#include "exceptions.hpp"

#include <drivers/x86/interrupt/Pic8259.hpp>
#include <drivers/x86/system/Idt.hpp>

#include <kernel/arch/x86/CpuState.hpp>
#include <kernel/assert.hpp>
#include <kernel/CriticalSection.hpp>
#include <kernel/debugSymbols.hpp>
#include <kernel/drivers.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/Log.hpp>
#include <kernel/logging.hpp>
#include <kernel/memory.hpp>
#include <kernel/panic.hpp>

#include <common/PodArray.hpp>
#include <common/types.hpp>

#include <atomic>

static Log log("arch::x86::exceptions");

namespace arch {
	namespace x86 {
		namespace exceptions {
			extern "C" void* _vectors[];

			PodArray<interrupt::Subscriber*> *interruptSubscribers[256] = {};
			PodArray<interrupt::Subscriber*> allInterruptSubscribers;

			#ifdef _64BIT
				// struct __attribute__((packed)) InterruptContext {
				// 	U64 r11;
				// 	U64 r10;
				// 	U64 r9;
				// 	U64 r8;
				// 	U64 rax;
				// 	U64 rcx;
				// 	U64 rdx;
				// 	U64 rsi;
				// 	U64 rdi;
				// 	U64 interrupt;
				// 	U64 error;

				// 	// Interrupt stack frame
				// 	U64 rip;
				// 	U64 cs;
				// 	U64 rflags;
				// 	U64 rsp;
				// 	U64 ss;
				// };
			#endif

			void handle_error(CpuState state, const char *error) {
				auto panic = panic::panic();

				panic.print_details_start();
					panic.print_details_inline("Error: ", error);

					#ifdef _64BIT
						if(state.interruptFrame.rip) panic.print_details_inline(" from ", to_string_hex(state.interruptFrame.rip));
					#else
						if(state.interruptFrame.eip) panic.print_details_inline(" from ", to_string_hex(state.interruptFrame.eip));
					#endif
					if(state.interruptFrame.error){
						panic.print_details_inline(" with error ", to_string_hex_trim(state.interruptFrame.error));
					}
					// if(reg.far) panic.print_details_inline(" touching ", to_string_hex(reg.far));
					// if(reg.esr) panic.print_details_inline(" with esr ", to_string_hex(reg.esr));
				panic.print_details_end();

				if(state.interrupt==0x0e){
					panic.print_details_start();
						if(state.interruptFrame.error&1<<0){
							panic.print_details_inline("  Page-protection violation");
						}else{
							panic.print_details_inline("  Page not present");
						}
						if(state.interruptFrame.error&1<<1){
							panic.print_details_inline(" when writing ");
						}else{
							panic.print_details_inline(" when reading ");
						}
						{
							U32 cr2;
							asm volatile("mov %0, cr2" : "=r"(cr2));
							panic.print_details_inline((void*)cr2);
						}
						if(state.interruptFrame.error&1<<2){
							panic.print_details_inline(" in userspace");
						}else{
							panic.print_details_inline(" in kernelspace");
						}
					panic.print_details_end();

					if(state.interruptFrame.error&1<<3){
						//TODO: reserved write (when PSE/PAE is set in CR4?)
					}
					if(state.interruptFrame.error&1<<4){
						panic.print_details_inline("  Instruction fetch");
					}
					if(state.interruptFrame.error&1<<5){
						panic.print_details_inline("  Protection-key violation");
					}
					if(state.interruptFrame.error&1<<6){
						panic.print_details_inline("  Shadow stack access");
					}
					if(state.interruptFrame.error&1<<15){
						panic.print_details_inline("  SGX violation");
					}
				}

					// U32 ebp;
					// U32 eax;
					// U32 ecx;
					// U32 edx;
					// U32 esi;
					// U32 edi;

				#ifdef _64BIT
				#else
					struct Stackframe {
						struct Stackframe* ebp;
						U32 eip;
					};

					auto &registers = state.registers;

					panic.print_details("");
					panic.print_details("  Registers:");
					panic.print_details("    eax = ", to_string_hex(registers.eax)/*, " ebx = ", to_string_hex(registers.ebx)*/, " ecx = ", to_string_hex(registers.ecx), " edx = ", to_string_hex(registers.edx));
					panic.print_details("    esi = ", to_string_hex(registers.esi), " edi = ", to_string_hex(registers.edi));
					panic.print_details("    esp = ", to_string_hex(registers.esp), " ebp = ", to_string_hex(registers.ebp));

					// TODO: set to stack of thread
					panic.print_stacktrace(registers.ebp);
				#endif
			}

			namespace interrupt {
				void subscribe(U8 vector, Subscriber callback) {
					auto &subscribers = interruptSubscribers[vector];
					if(!subscribers){
						subscribers = new PodArray<Subscriber*>(1);
					}
					subscribers->push_back(&callback);
				}

				void unsubscribe(U8 vector, Subscriber callback) {
					auto &subscribers = interruptSubscribers[vector];
					if(subscribers){
						for(auto i=0u;i<subscribers->length;i++){
							if((*subscribers)[i]==&callback){
								subscribers->remove(i);
								break;
							}
						}
					}
				}

				void subscribe_all(Subscriber callback) {
					allInterruptSubscribers.push_back(&callback);
				}

				void unsubscribe_all(Subscriber callback) {
					for(auto i=0u;i<allInterruptSubscribers.length;i++){
						if(allInterruptSubscribers[i]==&callback){
							allInterruptSubscribers.remove(i);
							break;
						}
					}
				}
			}

			const char *interruptErrorNames[21] = {
				"Divide error",
				"Debug exception",
				"NMI Interrupt",
				"Breakpoint",
				"Overflow",
				"Bounds check",
				"Illegal instruction",
				"FPU exception",
				"Double fault",
				"Coprocessor segment overrun",
				"Invalid TSS",
				"Segment not present",
				"Stack exception",
				"General protection fault",
				"Page fault",
				"Unknown error", // reserved by intel
				"Floating-point error",
				"Alignment check",
				"Machine check",
				"SIMD floating-point exception",
				"Virtualisation exception",
			};

			extern "C" const CpuState* _on_interrupt(const CpuState &state) {
				CriticalSection guard;

				// See if a global subscriber processes this..
				for(auto subscriber:allInterruptSubscribers) {
					if(auto outputState = (*subscriber)(state)){
						return outputState;
					}
				}

				// See if a specific subsciber processes this..
				if(auto subscribers = interruptSubscribers[state.interrupt]){
					for(auto subscriber:*subscribers){
						if(auto outputState = (*subscriber)(state)){
							return outputState;
						}
					}
				}

				if(auto outputState = (CpuState*)drivers::_on_interrupt(state.interrupt, &state)){
					return outputState;
				}

				// ..default handling if not processed
				switch(state.interrupt){
					case 0 ... 20:
						handle_error(state, interruptErrorNames[state.interrupt]);
					break;
				}

				return &state;
			}

			void init() {
				auto section = log.section("init...");

				auto idt = drivers::find_and_activate<driver::system::Idt>();
				assert(idt);

				//TODO: set these numbers intelligently. Maybe just set the built in ISA 0-15 here, and add the others on demand when subscribe_interrupt is called?
				for(auto i=0u;i<48;i++){
					idt->set_gate_interrupt(i, _vectors[i]);
				}

				idt->apply_gates();

				::exceptions::_activate();
			}
		}
	}
}

namespace exceptions {
	namespace interrupt {
		void subscribe(U8 vector, Subscriber callback) {
			arch::x86::exceptions::interrupt::subscribe(vector, (arch::x86::exceptions::interrupt::Subscriber)callback);
		}
		void unsubscribe(U8 vector, Subscriber callback) {
			arch::x86::exceptions::interrupt::unsubscribe(vector, (arch::x86::exceptions::interrupt::Subscriber)callback);
		}
		void subscribe_all(Subscriber callback) {
			arch::x86::exceptions::interrupt::subscribe_all((arch::x86::exceptions::interrupt::Subscriber)callback);
		}
		void unsubscribe_all(Subscriber callback) {
			arch::x86::exceptions::interrupt::unsubscribe_all((arch::x86::exceptions::interrupt::Subscriber)callback);
		}
	}

	void init() {
		arch::x86::exceptions::init();
		enable();
	}
}
