#pragma once

#include <common/types.hpp>

enum struct IpcId:U32 {
	init,

	start_main,

	custom = 0x8000'0000,
	custom_max
};
