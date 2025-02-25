#pragma once

#include <kernel/drivers/Hardware.hpp>

#include <common/EventEmitter.hpp>
#include <common/graphics2d/Buffer.hpp>
#include <common/graphics2d/BufferFormat.hpp>
#include <common/Try.hpp>

namespace driver {
	//TODO: should graphics drivers also include an api for querying their active processor(s) drivers if present? This would allow us to work out what processor speeds and temps relate to this graphics adapter, which might be useful/neat
	struct Graphics: Hardware {
		DRIVER_TYPE(Graphics, 0x2d7d3b85, "graphics", "Graphics Hardware", Hardware);

		struct Mode {
			U32 width;
			U32 height;
			graphics2d::BufferFormat format;
		};

		struct Event {
			Graphics *instance;

			enum struct Type {
				framebufferChanging, // framebuffer is currently invalid. Don't render to it. It will resolve with a framebufferChanged soon
				framebufferChanged
			} type;

			union {
				struct {
					U32 index;
				} framebufferChanging;

				struct {
					U32 index;
				} framebufferChanged;
			};
		};

		static inline EventEmitter<Event> allEvents;
		EventEmitter<Event> events;

		virtual auto set_mode(U32 framebufferId, U32 width, U32 height, graphics2d::BufferFormat format, bool acceptSuggestion = true) -> Try<>;

		virtual auto get_mode_count(U32 framebufferId) -> U32 = 0;
		virtual auto get_mode(U32 framebufferId, U32 index) -> Mode = 0;
		virtual auto set_mode(U32 framebufferId, U32 index) -> Try<> = 0;
		virtual auto detect_default_mode(U32 framebufferId) -> bool { return false; }
		virtual auto get_default_mode(U32 framebufferId) -> Mode { return { 0 }; }

		virtual auto get_framebuffer_count() -> U32 = 0;
		virtual auto get_framebuffer(U32 index) -> graphics2d::Buffer* = 0; // these may temporarily be null when switching mode - This is expected. Assume they are still valid screenspace, just don't render to them while nullptr
		virtual auto get_framebuffer_name(U32 index) -> const char* = 0;
	};
}
