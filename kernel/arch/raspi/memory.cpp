#include "memory.hpp"

#include <kernel/arch/raspi/atags.hpp>
#include <kernel/arch/raspi/hwquery.hpp>
#include <kernel/arch/raspi/memory.hpp>
#include <kernel/kernel.h>
#include <kernel/log.hpp>

#include <common/LList.hpp>
#include <common/MemoryPool.hpp>
#include <common/stdlib.hpp>
#include <common/types.hpp>

#include <new>

extern U8 __text_start;
extern U8 __text_end;
extern U8 __data_start;
extern U8 __data_end;
extern U8 __rodata_start;
extern U8 __rodata_end;
extern U8 __bss_start;
extern U8 __bss_end;
extern U8 __end;

namespace memory {
	extern Page *pageData;
	extern U32 pageDataSize;
	extern LList<Page> freePages;
	// extern MemoryPool<32> *heap;
}


namespace arch {
	namespace raspi {			
		namespace memory {
			using ::memory::freePages;
			using ::memory::pageData;
			using ::memory::pageDataSize;
			using ::memory::pageSize;
			using ::memory::totalMemory;
			using ::memory::Page;

			void init() {
				log::Section section("arch::raspi::memory::init...");

				if(!totalMemory){
					#if defined(ARCH_RASPI1)
						totalMemory = 1024*1024*128; //could be between 128 and 255, depending on gpu split

					#elif defined(ARCH_RASPI2)
						totalMemory = 1024*1024*1024;

					#elif defined(ARCH_RASPI3)
						totalMemory = 1024*1024*1024;

					#elif defined(ARCH_RASPI4)
						totalMemory = 1024*1024*1024; //could be 1, 2, 4 or 8GB

					#else
						#error "Unknown model"
					#endif

					log::print_warning("Warning: No memory size specified. Assuming ", totalMemory/1024/1024, "MB");
				}

				log::print_info("total memory: ", totalMemory/1024/1024, "MB");
				log::print_info("kernel stack: ", stackSize/1024, "KB");
				// log::print("kernel heap: ", heapSize/1024, "KB\n");

				log::print_info("page size: ", pageSize/1024, "KB");

				#pragma GCC diagnostic push
				#pragma GCC diagnostic ignored "-Warray-bounds"
					// heap = (MemoryPool<32>*)((U8*)&__end)+stackSize;
					auto kernelEnd = ::memory::heap + ::memory::heap_size;
				#pragma GCC diagnostic pop

				// { //initialise heap
				// 	bzero(heap, heapSize); //clear to be safe
				// 	new (heap) MemoryPool<32>(((U8*)heap)+sizeof(MemoryPool<32>), heapSize);
				// }

				auto pageCount = totalMemory / pageSize;
				pageData = (Page*)kernelEnd;
				pageDataSize = (pageCount * sizeof(Page) + pageSize-1) / pageSize * pageSize;

				auto kernelPageCount = ((size_t)kernelEnd+pageDataSize+pageSize-1) / pageSize;

				if(kernelPageCount>pageCount) pageCount = kernelPageCount;
				auto vramPageCount = (hwquery::videoMemory+pageSize-1) / pageSize;
				auto userPageCount = pageCount-kernelPageCount-vramPageCount;

				log::print_debug("kernel start @ ", &__end);
				log::print_debug("kernel end @ ", kernelEnd);

				log::print_debug("text @ ", &__text_start, " - ", &__text_end);
				log::print_debug("rodata @ ", &__rodata_start, " - ", &__rodata_end);
				log::print_debug("data @ ", &__data_start, " - ", &__data_end);
				log::print_debug("bss @ ", &__bss_start, " - ", &__bss_end);

				log::print_info("pages: ", pageCount);
				log::print_info(kernelPageCount, " kernel pages");
				log::print_info(vramPageCount, " vram pages");
				log::print_info(userPageCount, " user pages");
				log::print_info("");

				{ // initialise pages
					log::Section section("Clearing pages...");

					auto i = 0u;

					{ // kernel pages
						// log::Section section("Clearing kernel...");

						// log::print_info_start();

						for(;i<kernelPageCount;i++) {
							// log::print("kernel page ", i, "\n");
							auto page = new (&pageData[i]) Page((void*)(i*pageSize));
							page->virtualAddress = i*pageSize;
							page->isAllocated = true;
							page->isKernel = true;
							page->hasNextPage = i+1<kernelPageCount;

							// log::print_inline('.');
						}

						// log::print_end();
					}

					{ // user pages
						// log::Section section("Clearing user...");

						// log::print_info_start();

						for(;i<pageCount;i++) {
							// log::print("user page ", i, "\n");
							const auto address = (void*)(i*pageSize);
							auto page = new (&pageData[i]) Page(address);
							page->hasNextPage = i+1<pageCount;

							if(address>=hwquery::videoMemoryStart&&address<(void*)((U64)hwquery::videoMemoryStart+hwquery::videoMemory)){
								page->isAllocated = true;
								page->isKernel = true; //technically vram, not kernel ¯\_(ツ)_/¯
								
							}else{
								freePages.push_back(*page);
							}
							// return;

							// log::print_inline('.');
						}

						// log::print_end();
					}
				}
			}
		}
	}
}
