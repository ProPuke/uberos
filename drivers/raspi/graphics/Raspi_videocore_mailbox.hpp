#pragma once

#include <drivers/Graphics.hpp>

#include <common/graphics2d/BufferFormat.hpp>
#include <common/Try.hpp>
#include <common/types.hpp>

namespace driver::graphics {
	struct Raspi_videocore_mailbox final: driver::Graphics {
		DRIVER_INSTANCE(Raspi_videocore_mailbox, 0x809d34ea, "videocore", "Raspi Videocore Firmware", driver::Graphics)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto set_mode(U32 framebufferId, U32 width, U32 height, graphics2d::BufferFormat format, bool acceptSuggestion = true) -> Try<> override;

		auto get_mode_count() -> U32 override;
		auto get_mode(U32 framebufferId, U32 index) -> Mode override;
		auto set_mode(U32 framebufferId, U32 index) -> Try<> override;
		
		auto detect_default_mode() -> bool override;
		auto get_default_mode() -> Mode override;

		auto get_framebuffer_count() -> U32 override;
		auto get_framebuffer(U32 index) -> graphics2d::Buffer* override;
		auto get_framebuffer_name(U32 index) -> const char* override;
	};
}
