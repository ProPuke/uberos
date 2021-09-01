#include "atags.hpp"

#include <common/types.hpp>
#include <common/stdlib.hpp>
#include <kernel/stdio.hpp>

namespace arch {
	namespace raspi {
		namespace atags {
			U32 mem_size = 0;

			void init(Atag *tag){
				stdio::Section section("arch::raspi::atags::init...");

				mem_size = 0;

				if(tag&&tag->tag==Tag::none){
					stdio::print_warning("Warning: No atag information provided");
				}

				for(;tag->tag!=Tag::none; tag=(Atag*)((U32*)tag)+tag->tag_size){
					stdio::print_info("tag: ", (U32)tag->tag);

					switch(tag->tag){
						case Tag::mem:
							mem_size = tag->memory.size;
							stdio::print_info("memory: ", mem_size/1024/1024, "MB");
						break;
						case Tag::videotext:
						case Tag::ramdisk:
						case Tag::initrd2:
							//TODO
						break;
						case Tag::serial: {
							U32 serialLow = tag->serialNumber.low;
							U32 serialHigh = tag->serialNumber.high;
							stdio::print_info("serial: ", serialLow, " ", serialHigh);
						} break;
						case Tag::revision:
						case Tag::videolfb:
						case Tag::cmdline:
							//TODO
						break;
						default:
							stdio::print_info("unknown tag: ", (U32)tag->tag);
						break;
					}
				}
			}
		}
	}
}
