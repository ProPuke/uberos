#include "framebuffer.hpp"

namespace framebuffer {
	Framebuffer framebuffers[framebuffer::max_framebuffers];
	unsigned framebuffer_count = 0;

	U32 default_resolution[2] = {0,0};
}
