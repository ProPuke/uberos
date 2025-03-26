#pragma once

#include "memory.hpp"

#include <kernel/memory/Page.hpp>

#include <common/types.hpp>

namespace memory {
	inline auto _get_physical_memory_page(Physical<void> physical) -> Physical<Page> {
		return Physical<Page>{physical.address/memory::pageSize*memory::pageSize};
	}
}
