#include <kernel/panic.hpp>

#include <kernel/debugSymbols.hpp>
#include <kernel/drivers.hpp>

namespace panic {
	struct Stackframe {
		struct Stackframe* ebp;
		U32 eip;
	};

	auto Panic::print_stacktrace() -> Panic& {
		UPtr ebp;
		asm volatile(
			"mov %0, ebp"
			: "=r"(ebp)
		);

		return print_stacktrace(ebp);
	}

	auto Panic::print_stacktrace(UPtr stackframeAddr) -> Panic& {
		if(!stackframeAddr) return *this;

		print_details("");
		print_details("Stacktrace:");
		{
			auto stackframe = (Stackframe*)stackframeAddr;
			DriverType *driverType = nullptr;
			for(U32 depth=0;stackframe&&depth<64;depth++){
				const auto stackBottom = memory::stack+memory::stackSize;
				const auto stackTop = memory::stack;

				if((UPtr)stackframe<stackTop.address||(UPtr)stackframe>=stackBottom.address){
					print_details("  - Connection lost (at ", to_string_hex((U32)(UPtr)stackframe), ')');
					break;
				}

				// sp = fp + 0x10;
				// fp = *(U32*)fp;

				// pc = *(U32*)(fp+8);
				auto pc = stackframe->eip;

				{ // eip points at the NEXT instruction, so roll it back one if we detect what looks like a call before

					if(((U8*)pc)[-5]==0xe8){ // direct near call
						pc -= 5;

					}else if(((U8*)pc)[-7]==0x9a){ // direct call
						pc -= 7;

					}else if(((U8*)pc)[-2]==0xff && ((U8*)pc)[-1]>>6==3 && ((U8*)pc)[-1]>>3 & 0x07==2){ // indirect call via register (with mod=3 and reg=2)
						pc -= 2;

					}else if(((U8*)pc)[-6]==0xff && ((U8*)pc)[-5]>>6==0 && ((U8*)pc)[-5]>>3 & 0x07==2 && ((U8*)pc)[-5] & 0x07==5){ // indirect call via memory (with mod=0, reg=2 and rm=5)
						pc -= 6;
					}
				}

				auto function = debugSymbols::get_function_by_address((void*)pc);

				if(function){
					if(function->driverType!=driverType||depth==0){
						driverType = function->driverType;

						if(depth>0){
							print_details("");
						}
						if(driverType){
							print_details_start();
							print_details_inline("  In ", driverType->name, " (", driverType->description, ')');
							for(auto parent = driverType->parentType; parent&&parent->parentType; parent = parent->parentType) {
								print_details_inline(" / ", parent->description);
							}
							print_details_end();
						}else{
							print_details("  In core:");
						}
					}
					print_details("    ", depth, " - ", to_string_hex(pc), " ", function->name, " + ", to_string_hex_trim(pc-(U64)function->address));
				}else{
					if(driverType){
						if(depth>0){
							print_details("");
						}
						driverType = nullptr;
						print_details("  In core:");
					}
					print_details("    ", depth, " - ", to_string_hex(pc));
				}

				stackframe = stackframe->ebp;
			}
		}

		return *this;
	}
}
