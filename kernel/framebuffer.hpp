#pragma once

#include "Framebuffer.hpp"
#include <common/types.hpp>

namespace framebuffer {
	static const unsigned max_framebuffers = 8;

	extern Framebuffer framebuffers[max_framebuffers];
	extern unsigned framebuffer_count;

	extern U32 default_resolution[2];

	bool set_mode(U32 framebufferId, U32 width, U32 height, FramebufferFormat format, bool acceptSuggestion = true);

	bool detect_default_resolution();
}
