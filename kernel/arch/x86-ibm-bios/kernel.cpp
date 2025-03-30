#include <kernel/arch/x86-ibm-bios/bios.hpp>
#include <kernel/arch/x86-ibm-bios/config.h>
#include <kernel/arch/x86-ibm/stdout.hpp>
#include <kernel/arch/x86/cpuInfo.hpp>
#include <kernel/arch/x86/nmi.hpp>
#include <kernel/Driver.hpp>
#include <kernel/drivers.hpp>
#include <kernel/drivers.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/kernel.hpp>
#include <kernel/logging.hpp>
#ifdef KERNEL_MMU
	#include <kernel/mmu.hpp>
#endif
#include <kernel/multiboot.hpp>

#include <common/format.hpp>

#include <lib/multiboot/multiboot1.h>

extern U8 __start, __end;

namespace {
	void init_commandline_parameter(const char *cmdline) {
		//TODO: if "serial_debug" is present, hook stdout to COM1, 8-N-1 57600 baud

		if(auto param=strstr(cmdline, "safemode");param&&(param==cmdline||param[-1]==' ')&&(param[8]==' '||param[8]=='\0')){
			kernel::isSafemode = true;
		}
	}

	void init_multiboot1(unsigned long magic, multiboot1_info *multiboot) {
		if(magic!=MULTIBOOT1_BOOTLOADER_MAGIC) return;

		::multiboot::multiboot1 = multiboot;

		{ // serial debug
			// cmdline passed?
			if(multiboot->flags & MULTIBOOT1_INFO_CMDLINE){
				init_commandline_parameter((const char*)multiboot->cmdline);
			}
		}

		{ // find memory
			U64 maxAddress = 0x0;

			// find the available memory the kernel sits within
			for(
				auto mmap=(multiboot1_memory_map_t*)multiboot->mmap_addr;
				mmap<mmap+multiboot->mmap_length;
				mmap = (multiboot1_memory_map_t*)((size_t)mmap + sizeof(mmap->size) + mmap->size)
			) {
				if(mmap->type!=MULTIBOOT1_MEMORY_AVAILABLE) continue;

				maxAddress = mmap->addr+mmap->len;

				if(mmap->addr<=(U64)&__start && mmap->addr+mmap->len>=(U64)&__end){
					memory::heapSize = (UPtr)(mmap->addr+mmap->len) - memory::heap.address;
					break;
				}
			}

			memory::totalMemory = maxAddress+1;
		}
	}

	void init_multiboot2(unsigned long magic, void *multiboot) {
		if(magic!=MULTIBOOT2_BOOTLOADER_MAGIC) return;

		if((UPtr)multiboot&7) return; //invalid alignment

		// auto size = *(U32*)multiboot;

		auto address = multiboot;
		for(auto tag = (multiboot2_tag*)((U8*)address+8); tag->type!=MULTIBOOT2_TAG_TYPE_END; tag = (multiboot2_tag*)((U8*)tag+ ((tag->size+7)&~7))){
			switch(tag->type){
				case MULTIBOOT2_TAG_TYPE_CMDLINE: {
					auto &info = *(multiboot2_tag_string*)tag;

					init_commandline_parameter(info.string);
				} break;
				case MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO: {
					// auto &info = *(multiboot2_tag_basic_meminfo*)tag;

					// memory::totalMemory = 1024*1024 + info.mem_upper;
				} break;
				case MULTIBOOT2_TAG_TYPE_MMAP: {
					auto &info = *(multiboot2_tag_mmap*)tag;

					U64 maxAddress = 0x0;

					for(auto entry = info.entries; (void*)entry < (U8*)&info + info.size; entry = (multiboot2_memory_map_t*)((U8*)entry + info.entry_size)){
						switch(entry->type){
							case MULTIBOOT2_MEMORY_AVAILABLE:
								maxAddress = entry->addr+entry->len;

								if(entry->addr<=(U64)&__start && entry->addr+entry->len>=(U64)&__end){
									memory::heapSize = (UPtr)(entry->addr+entry->len) - memory::heap.address;
									goto memoryFound;
								}
							break;
							case MULTIBOOT2_MEMORY_RESERVED:
							case MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE:
							case MULTIBOOT2_MEMORY_NVS:
							case MULTIBOOT2_MEMORY_BADRAM:
							break;
						}
					}

					memoryFound:

					memory::totalMemory = maxAddress+1;
				} break;
				case MULTIBOOT2_TAG_TYPE_FRAMEBUFFER: {
					auto &info = *(multiboot2_tag_framebuffer*)tag;
					multiboot::multiboot2_framebuffer = &info;
				} break;
			}
		}
	}
}

namespace arch {
	namespace x86_ibm_bios {
		namespace kernel {
			extern "C" void entrypoint(unsigned long magic, void *multiboot) {
				::arch::x86::cpuInfo::enable_sse(); //mmx/sse instructions may be present in startup code, so get this in early
				::arch::x86::nmi::disable();
				::exceptions::disable();

				init_multiboot1(magic, (multiboot1_info*)multiboot);
				init_multiboot2(magic, multiboot);

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
		// #ifdef KERNEL_MMU
		// 	mmu::init();
		// #endif
	}

	void _postInit() {
		
	}
}
