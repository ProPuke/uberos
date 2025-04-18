#pragma once

#include <drivers/Graphics.hpp>

namespace driver::graphics {
	struct BochsVga final: driver::Graphics {
		DRIVER_INSTANCE(BochsVga, 0x99815895, "bochsVga", "Bochs VGA", driver::Graphics)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto get_mode_count(U32 framebufferId) -> U32 override;
		auto get_mode(U32 framebufferId, U32 index) -> Mode override;
		auto set_mode(U32 framebufferId, U32 index) -> Try<> override;
		auto set_mode(U32 framebufferId, U32 width, U32 height, graphics2d::BufferFormat format, bool acceptSuggestion = true) -> Try<> override;
		
		auto get_framebuffer_count() -> U32 override;
		auto get_framebuffer(U32 index) -> graphics2d::Buffer* override;
		auto get_framebuffer_name(U32 index) -> const char* override;
	};
}
