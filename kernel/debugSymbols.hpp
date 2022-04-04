#pragma once

#include <common/types.hpp>

#include "debugSymbols.h"

namespace debugSymbols {
	auto get_symbol_by_address(void *address) -> DebugSymbol*;
}
