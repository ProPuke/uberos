#pragma once

#include <kernel/Driver.hpp>
#include <kernel/framebuffer.hpp>
#include <kernel/Framebuffer.hpp>

namespace driver {
	struct Graphics: Driver {
		/**/ Graphics(U64 address, const char *name, const char *descriptiveType):
			Driver(address, name, "graphics", descriptiveType)
		{}

		virtual auto set_mode(U32 framebufferId, U32 width, U32 height, FramebufferFormat format, bool acceptSuggestion = true) -> bool = 0;

		virtual auto get_mode_count() -> U32 = 0;
		virtual auto get_mode(U32 framebufferId, U32 index) -> framebuffer::Mode = 0;
		virtual auto detect_default_mode() -> bool { return false; }
		virtual auto get_default_mode() -> framebuffer::Mode = 0;

		virtual auto get_framebuffer_count() -> U32 = 0;
		virtual auto get_framebuffer(U32 index) -> Framebuffer* = 0;
	};
}