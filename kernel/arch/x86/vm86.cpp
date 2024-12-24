#include "vm86.hpp"

#include <kernel/arch/x86/CpuState.hpp>
#include <kernel/arch/x86/exceptions.hpp>
#include <kernel/CriticalSection.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/Log.hpp>

#include <common/Box.hpp>

static Log log("arch::x86::vm86");

extern "C" U8 _vm86_call_bios_start;
extern "C" U8 _vm86_call_bios_end;

namespace arch {
	namespace x86 {
		namespace vm86 {
			// struct __attribute__((packed)) FarPtr {
			// 	/**/ FarPtr(void *address) {
			// 		debug::assert((size_t)address<0x100000); // must be under 1MB

			// 		offset = (size_t)address&0xffff;
			// 		segment = ((size_t)address-offset) >> 4;
			// 	}

			// 	U16 segment; // 16 byte block
			// 	U16 offset;
			// };

			// auto create_task(void *codeStart, size_t codeLength, void *pageDirectory, size_t kernelStackSize, size_t userStackSize, CpuState::GeneralRegisters &registers) {
			// 	CriticalSection guard;

			// 	vmm_modify_page_directory(page_directory?:vmm_create_page_directory());
			// 	/// Identity maps the first MiB so our VM86 task can operate inside it.
			// 	/// Because this is not inside the user domain (1GiB and onwards),
			// 	/// bypasses domain checking.
			// 	vmm_enable_domain_check(0);
			// 	vmm_map_range(0, 0, MULTIBOOT_LOWER_MEMORY, VMM_USER|VMM_WRITABLE);
			// 	vmm_enable_domain_check(1);

			// 	auto vmCodeAddress = (void*)0x500; //place code at the beginning of conventional memory for now

			// 	// copy the code into conventional mem for now. Note that this will break position-dependent code
			// 	memcpy(vmCodeAddress, codeStart, codeLength);
			// 	Box<U8> kernelStack = new U8[kernelStackSize];

			// 	// place stack after the code
			// 	auto userStack = codeStart+codeLength;

			// 	auto &cpu = *(CpuState*)(kernelStack+kernelStackSize-1-sizeof(CpuState));

			// 	// the segment registers are overwritten with the vm86_* values when iret'ing
			// 	auto selector = gdt_get_selector(GDT_RING3_DATA_SEG);
			// 	cpu.segments.gs = selector;
			// 	cpu.segments.fs = selector;
			// 	cpu.segments.es = selector;
			// 	cpu.segments.ds = selector;
			// 	cpu.registers = registers;

			// 	// set realmode address of code
			// 	FarPtr entrypointFarPtr(vmCodeAddress);
			// 	cpu.cs  = entrypointFarPtr.segment;
			// 	cpu.eip = entrypointFarPtr.offset;

			// 	cpu.eflags.dword         = 0;
			// 	cpu.eflags.bits._if      = 1;
			// 	cpu.eflags.bits.reserved = 1;
			// 	cpu.eflags.bits.vm       = 1; // gogo virtual 8086 mode!

			// 	FarPtr user_stack_farptr((void*)((U32)userStack + userStackSize - 1));

			// 	// set stackpointer in realmode
			// 	cpu.userspace.esp = user_stack_farptr.offset;
			// 	cpu.userspace.ss  = user_stack_farptr.segment;

			// 	// code and stack in the same segment
			// 	cpu.vm86.es = entrypointFarPtr.segment;
			// 	cpu.vm86.ds = entrypointFarPtr.segment;
			// 	cpu.vm86.fs = entrypointFarPtr.segment;
			// 	cpu.vm86.gs = entrypointFarPtr.segment;

			// 	vmm_modified_page_directory();

			// 	return task;
			// }

			// void call_bios(U8 interrupt, CpuState::GeneralRegisters &registers) {
			// 	auto opcode = (U8*)&_vm86_interrupt_hook; // the interrupt opcode
			// 	auto operand = opcode+1; // the actual interrupt vector

			// 	if(*opcode != Opcode::_int) {
			// 		log.print_error("Error: BIOS handler corrupted");
			// 		debug::halt();
			// 		return;
			// 	}

			// 	*operand = interrupt;

			// 	create_task(&_vm86_call_bios_start, &_vm86_call_bios_end-&_vm86_call_bios_start, 0x0, 0x1000, 0x1000, registers);
			// }

			static const CpuState* _on_generalProtectionFault(const CpuState &cpuState) {
				return nullptr;
				// if(!vm86_monitor(cpuState)) return nullptr;
				// return &cpuState;
			}

			void init() {
				arch::x86::exceptions::interrupt::subscribe(0x0d, _on_generalProtectionFault);
			}
		}
	}
}
