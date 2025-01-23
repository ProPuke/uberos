#include "atags.hpp"

#include <kernel/memory.hpp>
#include <kernel/Log.hpp>

#include <common/stdlib.hpp>
#include <common/types.hpp>

static Log log("arch::raspi::atags");

namespace arch {
	namespace raspi {
		namespace atags {
			U32 mem_size = 0;

			void init(const Atag *tag){
				auto section = log.section("init...");

				mem_size = 0;

				if(tag&&tag->tag==Tag::none){
					log.print_warning("Warning: No atag information provided");
				}

				auto aborted = false;

				for(;tag->tag!=Tag::none; tag=(Atag*)((U32*)tag)+tag->tag_size){
					// log.print_info("tag: ", (U32)tag->tag);

					switch(tag->tag){
						case Tag::none: break;
						case Tag::mem: {
							const U64 totalMemory = tag->memory.size;
							log.print_info("memory: ", totalMemory/1024/1024, "MB");
							memory::totalMemory = totalMemory;
						} break;
						case Tag::videotext:
						case Tag::ramdisk:
						case Tag::initrd2:
							//TODO
						break;
						case Tag::serial: {
							U32 serialLow = tag->serialNumber.low;
							U32 serialHigh = tag->serialNumber.high;
							log.print_info("serial: ", serialLow, " ", serialHigh);
						} break;
						case Tag::revision:
						case Tag::videolfb:
						case Tag::cmdline:
							//TODO
						break;
						default:
							log.print_error("unknown tag: ", (U32)tag->tag);
							aborted = true;
						break;
					}

					if(aborted) break;
				}
			}
		}
	}
}
