#include <kernel/arch/x86-ibm-bios/bios.hpp>
#include <kernel/arch/x86-ibm-bios/config.h>
#include <kernel/arch/x86-ibm-bios/framebuffer.hpp>
#include <kernel/arch/x86-ibm/stdout.hpp>
#include <kernel/arch/x86/acpi.hpp>
#include <kernel/arch/x86/cpu.hpp>
#include <kernel/arch/x86/cpuInfo.hpp>
#include <kernel/arch/x86/gdt.hpp>
#include <kernel/arch/x86/timer.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/kernel.hpp>
#include <kernel/drivers.hpp>
#include <kernel/mmu.hpp>

#include <lib/multiboot/multiboot.h>

namespace arch {
	namespace x86_ibm_bios {
		namespace kernel {
			extern "C" void entrypoint(unsigned long magic, multiboot_info *multiboot) {
				::arch::x86::cpuInfo::enable_sse(); //mmx/sse instructions may be present in startup code, so get this in early

				if(magic==MULTIBOOT_BOOTLOADER_MAGIC){
					// cmdline passed?
					if(multiboot->flags & 1<<2){
						//TODO: if "serial_debug" is present, hook stdout to COM1, 8-N-1 57600 baud
						(void)multiboot->cmdline;
					}
				}

				::kernel::run();
			}
		}
	}
}

namespace kernel {
	void _preInit() {
		arch::x86_ibm::stdout::init();
		arch::x86::gdt::init();
		exceptions::init();
		arch::x86_ibm_bios::bios::init();
		arch::x86::acpi::init();
		arch::x86::cpu::init();
		debug::halt();
		arch::x86_ibm_bios::framebuffer::init();
		#ifdef KERNEL_MMU
			mmu::init();
		#endif
		arch::x86::timer::init();
	}

	void _postInit() {
		
	}
}
