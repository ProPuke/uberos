#pragma once

#include "Framebuffer.hpp"
#include <common/types.hpp>

namespace framebuffer {
	struct Mode {
		U32 width;
		U32 height;
		FramebufferFormat format;
	};

	void init();

	auto get_framebuffer_count() -> U32;
	auto get_framebuffer(U32 index) -> Framebuffer*;
}
