#include <kernel/arch/x86-ibm-bios/bios.hpp>
#include <kernel/arch/x86-ibm-bios/config.h>
#include <kernel/arch/x86-ibm/stdout.hpp>
#include <kernel/arch/x86/cpuInfo.hpp>
#include <kernel/Driver.hpp>
#include <kernel/drivers.hpp>
#include <kernel/drivers.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/kernel.hpp>
#include <kernel/logging.hpp>
#include <kernel/mmu.hpp>
#include <kernel/multiboot.hpp>

#include <common/format.hpp>

#include <lib/multiboot/multiboot.h>

extern U8 __start;
extern U8 __end;

namespace {
	void init_multiboot(unsigned long magic, multiboot_info *multiboot) {
		if(magic!=MULTIBOOT_BOOTLOADER_MAGIC) return;

		::multiboot::multiboot1 = multiboot;

		{ // serial debug
			// cmdline passed?
			if(multiboot->flags & MULTIBOOT_INFO_CMDLINE){
				//TODO: if "serial_debug" is present, hook stdout to COM1, 8-N-1 57600 baud
				auto cmdline = (const char*)multiboot->cmdline;

				if(auto param=strstr(cmdline, "safemode");param&&(param==cmdline||param[-1]==' ')&&(param[8]==' '||param[8]=='\0')){
					kernel::isSafemode = true;
				}
			}
		}

		{ // find memory
			U64 maxAddress = 0x0;

			// find the available memory the kernel sits within
			for(
				auto mmap=(multiboot_memory_map_t*)multiboot->mmap_addr;
				mmap<mmap+multiboot->mmap_length;
				mmap = (multiboot_memory_map_t*)((size_t)mmap + sizeof(mmap->size) + mmap->size)
			) {
				maxAddress = mmap->addr+mmap->len;

				if(mmap->type!=MULTIBOOT_MEMORY_AVAILABLE) continue;

				if(mmap->addr<=(U64)&__start && mmap->addr+mmap->len>=(U64)&__end){
					memory::heapSize = (U8*)(mmap->addr+mmap->len) - (U8*)memory::heap;
					break;
				}
			}

			memory::totalMemory = maxAddress+1;
		}
	}
}

namespace arch {
	namespace x86_ibm_bios {
		namespace kernel {
			extern "C" void entrypoint(unsigned long magic, multiboot_info *multiboot) {
				::arch::x86::cpuInfo::enable_sse(); //mmx/sse instructions may be present in startup code, so get this in early

				init_multiboot(magic, multiboot);

				::kernel::run();
			}
		}
	}
}

namespace kernel {
	void _preInit() {
		if(isSafemode){
			arch::x86_ibm::stdout::init();
		}
		exceptions::init();
		#ifdef KERNEL_MMU
			mmu::init();
		#endif
	}

	void _postInit() {
		
	}
}
