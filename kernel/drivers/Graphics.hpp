#pragma once

#include <kernel/Driver.hpp>
#include <kernel/framebuffer.hpp>
#include <kernel/Framebuffer.hpp>

#include <common/graphics2d/BufferFormat.hpp>

namespace driver {
	//TODO: should graphics drivers also include an api for querying their active processor(s) drivers if present? This would allow us to work out what processor speeds and temps relate to this graphics adapter, which might be useful/neat
	struct Graphics: Driver {
		typedef Driver Super;

		static DriverType driverType;

		/**/ Graphics(const char *name, const char *description);

		virtual auto set_mode(U32 framebufferId, U32 width, U32 height, graphics2d::BufferFormat format, bool acceptSuggestion = true) -> bool;

		virtual auto get_mode_count(U32 framebufferId) -> U32 = 0;
		virtual auto get_mode(U32 framebufferId, U32 index) -> framebuffer::Mode = 0;
		virtual auto set_mode(U32 framebufferId, U32 index) -> bool = 0;
		virtual auto detect_default_mode(U32 framebufferId) -> bool { return false; }
		virtual auto get_default_mode(U32 framebufferId) -> framebuffer::Mode { return { 0 }; }

		virtual auto get_framebuffer_count() -> U32 = 0;
		virtual auto get_framebuffer(U32 index) -> Framebuffer* = 0;
		virtual auto get_framebuffer_name(U32 index) -> const char* = 0;
	};
}
