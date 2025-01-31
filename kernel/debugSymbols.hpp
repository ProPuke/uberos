#pragma once

#include <kernel/Driver.hpp>

#include <common/types.hpp>

#include "debugSymbols.h"

namespace debugSymbols {
	struct Function {
		const char *name;
		void *address;
		unsigned size;
		DriverType *driverType;
	};

	auto get_function_by_address(void *address) -> Function*;
}
