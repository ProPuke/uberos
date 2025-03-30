#pragma once

#include <lib/multiboot/multiboot1.h>
#include <lib/multiboot/multiboot2.h>

namespace multiboot {
	inline constinit multiboot1_info *multiboot1 = nullptr;
	inline constinit multiboot2_tag_framebuffer *multiboot2_framebuffer = nullptr;
}
