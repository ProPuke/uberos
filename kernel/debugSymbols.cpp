#include "debugSymbols.hpp"

extern "C" DebugSymbol debugSymbolsArray[];

namespace debugSymbols {
	auto get_symbol_by_address(void *address) -> DebugSymbol* {
		DebugSymbol *closest = nullptr;
		U64 closest_distance = 0;
		for(auto symbol = &::debugSymbolsArray[0]; symbol->name[0]; symbol++){
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
