#include "memory.hpp"

#include <common/types.hpp>
#include <common/LList.hpp>
#include "atags.hpp"
#include "memory.hpp"
#include "hwquery.hpp"
#include <common/stdlib.hpp>
#include <kernel/stdio.hpp>
#include <common/MemoryPool.hpp>
#include <new>

extern U8 __end;

namespace memory {
	extern Page *pageData;
	extern U32 pageDataSize;
	extern LList<Page> freePages;
	// extern MemoryPool<32> *heap;
}


namespace atags {
	using namespace arch::raspi::atags;
}

namespace systemInfo {
	using namespace arch::raspi;
}

namespace hwquery {
	using namespace arch::raspi;
}

namespace memory {
	namespace arch {
		namespace raspi {			
			void init() {
				stdio::Section section("memory::arch::raspi::init...");

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

					stdio::print_warning("Warning: No memory size specified. Assuming ", totalMemory/1024/1024, "MB");
				}

				stdio::print_info("total memory: ", totalMemory/1024/1024, "MB");
				stdio::print_info("kernel stack: ", stackSize/1024, "KB");
				// stdio::print("kernel heap: ", heapSize/1024, "KB\n");

				stdio::print_info("page size: ", pageSize/1024, "KB");

				#pragma GCC diagnostic push
				#pragma GCC diagnostic ignored "-Warray-bounds"
					// heap = (MemoryPool<32>*)((U8*)&__end)+stackSize;
					auto kernelEnd = ((U8*)&__end)+stackSize;//+heapSize;
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

				// stdio::print_debug("kernel start @ ", &__end);
				// stdio::print_debug("kernel end @ ", kernelEnd);

				stdio::print_info("pages: ", pageCount);
				stdio::print_info(kernelPageCount, " kernel pages");
				stdio::print_info(vramPageCount, " vram pages");
				stdio::print_info(userPageCount, " user pages");
				stdio::print_info("");

				{ // initialise pages
					stdio::Section section("Clearing pages...");

					auto i = 0u;

					{ // kernel pages
						// stdio::Section section("Clearing kernel...");

						// stdio::print_info_start();

						for(;i<kernelPageCount;i++) {
							// stdio::print("kernel page ", i, "\n");
							auto page = new (&pageData[i]) Page((void*)(i*pageSize));
							page->virtualAddress = i*pageSize;
							page->isAllocated = true;
							page->isKernel = true;
							page->hasNextPage = i+1<kernelPageCount;

							// stdio::print_inline('.');
						}

						// stdio::print_end();
					}

					{ // user pages
						// stdio::Section section("Clearing user...");

						// stdio::print_info_start();

						for(;i<pageCount;i++) {
							// stdio::print("user page ", i, "\n");
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

							// stdio::print_inline('.');
						}

						// stdio::print_end();
					}
				}
			}
		}
	}
}
