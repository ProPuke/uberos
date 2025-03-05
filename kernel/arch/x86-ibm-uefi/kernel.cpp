#include <kernel/arch/uefi/uefi.hpp>
#include <kernel/arch/x86-ibm/stdout.hpp>
#include <kernel/arch/x86/acpi.hpp>
#include <kernel/arch/x86/cpu.hpp>
#include <kernel/arch/x86/cpuInfo.hpp>
#include <kernel/arch/x86/gdt.hpp>
#include <kernel/arch/x86/timer.hpp>
#include <kernel/drivers.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/kernel.hpp>
#include <kernel/mmu.hpp>

namespace arch::x86_ibm_uefi::kernel {
	__attribute__((stdcall)) uefi::Status entrypoint(uefi::Handle imageHandle, uefi::SystemTable *systemTable) {
		uefi::init(systemTable);
		::kernel::run();
		return 0;
	}
}

namespace kernel {
	void _preInit() {
		if(isSafemode){
			arch::x86_ibm::stdout::init();
		}
		arch::x86::gdt::init();
		exceptions::init();
		arch::x86::acpi::init();
		arch::x86::cpu::init();
		#ifdef KERNEL_MMU
			mmu::init();
		#endif
		arch::x86::timer::init();
	}

	void _postInit() {
		
	}
}
