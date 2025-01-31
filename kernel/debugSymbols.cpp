#include "debugSymbols.hpp"

namespace debugSymbols {
	extern Function functionsArray[];

	auto get_function_by_address(void *address) -> Function* {
		Function *closest = nullptr;
		U64 closest_distance = 0;
		for(auto symbol = &functionsArray[0]; symbol->name; symbol++){
			if(address<symbol->address||symbol->size&&address>=(U8*)symbol->address+symbol->size) continue;

			U64 distance = (U8*)address-(U8*)symbol->address;
			if(!closest||distance<closest_distance){
				closest = symbol;
				closest_distance = distance;
			}
		}

		return closest;
	}
}
