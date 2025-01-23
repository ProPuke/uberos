#include <kernel/mmu.hpp>
#include <kernel/logging.hpp>
#include <kernel/kernel.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/drivers.hpp>
#include <kernel/drivers.hpp>
#include <kernel/Driver.hpp>
#include <kernel/arch/x86/timer.hpp>
#include <kernel/arch/x86/cpuInfo.hpp>
#include <kernel/arch/x86-ibm/stdout.hpp>
#include <kernel/arch/x86-ibm-bios/config.h>
#include <kernel/arch/x86-ibm-bios/bios.hpp>

#include <common/format.hpp>

#include <lib/multiboot/multiboot.h>

extern U8 __start;
extern U8 __end;

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

				::kernel::run();
			}
		}
	}
}

namespace kernel {
	void _preInit() {
		arch::x86_ibm::stdout::init();
		exceptions::init();
		// for(auto &driver:drivers::iterate<Driver>()){
		// 	logging::print_info_start();
		// 	logging::print_inline(driver.type->name, " (", driver.type->description, ")");
		// 	for(auto parent = driver.type->parentType; parent&&parent->parentType; parent = parent->parentType) {
		// 		logging::print_inline(" / ", parent->description);
		// 	}
		// 	logging::print_end();
		// 	// logging::print_info(driver.type->name, " (", driver.type->description, ") - ", driver.type->pretty_typename());
		// }
		#ifdef KERNEL_MMU
			mmu::init();
		#endif
		arch::x86::timer::init();
		// debug::halt();
	}

	void _postInit() {
		
	}
}
