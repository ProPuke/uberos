#include "exceptions.hpp"

#include <kernel/arch/x86/CpuState.hpp>
#include <kernel/assert.hpp>
#include <kernel/CriticalSection.hpp>
#include <kernel/debugSymbols.hpp>
#include <kernel/drivers.hpp>
#include <kernel/drivers/x86/interrupt/Pic8259.hpp>
#include <kernel/drivers/x86/system/Idt.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/Log.hpp>
#include <kernel/logging.hpp>
#include <kernel/memory.hpp>
#include <kernel/PodArray.hpp>

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
				logging::print_error_start();
					logging::print_inline("Error: ", error);

					#ifdef _64BIT
						if(state.interruptFrame.rip) logging::print_inline(" from ", to_string_hex(state.interruptFrame.rip));
					#else
						if(state.interruptFrame.eip) logging::print_inline(" from ", to_string_hex(state.interruptFrame.eip));
					#endif
					if(state.interruptFrame.error){
						logging::print_inline(" with error ", to_string_hex_trim(state.interruptFrame.error));
					}
					// if(reg.far) logging::print_inline(" touching ", to_string_hex(reg.far));
					// if(reg.esr) logging::print_inline(" with esr ", to_string_hex(reg.esr));
				logging::print_end();

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

					logging::print_error("  Registers:");
					logging::print_error("    eax = ", to_string_hex(registers.eax)/*, " ebx = ", to_string_hex(registers.ebx)*/, " ecx = ", to_string_hex(registers.ecx), " edx = ", to_string_hex(registers.edx));
					logging::print_error("    esi = ", to_string_hex(registers.esi), " edi = ", to_string_hex(registers.edi));
					logging::print_error("    esp = ", to_string_hex(registers.esp), " ebp = ", to_string_hex(registers.ebp));

					if(registers.ebp){
						logging::print_error("  Stacktrace:");

						auto stackFrame = (Stackframe*)registers.ebp;
						for(U32 depth=0;stackFrame&&depth<64;depth++){
							// TODO: set to stack of thread
							const auto stackBottom = (U8*)memory::stack+memory::stackSize;
							const auto stackTop = (U8*)memory::stack;

							if((void*)stackFrame<stackTop||(void*)stackFrame>=stackBottom){
								logging::print_error("    - Connection lost (at ", to_string_hex((unsigned)(size_t)stackFrame), ')');
								break;
							}else if((U32)stackFrame&0xf){
								logging::print_error("    - Non-aligned ebp (at ", to_string_hex((unsigned)(size_t)stackFrame), ')');
								break;
							}

							// sp = fp + 0x10;
							// fp = *(U32*)fp;

							// pc = *(U32*)(fp+8);
							auto pc = stackFrame->eip;

							auto symbol = debugSymbols::get_symbol_by_address((void*)pc);

							if(symbol){
								logging::print_error("    ", depth, " - ", to_string_hex(pc), " ", symbol->name, " + ", to_string_hex_trim(pc-(U64)symbol->address));
							}else{
								logging::print_error("    ", depth, " - ", to_string_hex(pc));
							}

							stackFrame = stackFrame->ebp;
						}
					}
				#endif

				halt();
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

				auto pic8259 = drivers::find_and_activate<driver::interrupt::Pic8259>();
				assert(pic8259);

				// pic8259->disable_all_irqs();

				auto idt = drivers::find_and_activate<driver::system::Idt>();
				assert(idt);

				for(auto i=0u;i<48;i++){
					idt->set_entry(i, _vectors[i], 0x8e);
				}

				idt->apply_entries();

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
	}
}
