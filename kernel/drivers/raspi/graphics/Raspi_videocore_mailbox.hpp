#include <kernel/drivers/Graphics.hpp>
#include <kernel/drivers.hpp>

#include <common/graphics2d/BufferFormat.hpp>
#include <common/Try.hpp>
#include <common/types.hpp>

namespace driver::graphics {
	struct Raspi_videocore_mailbox final: driver::Graphics {
		DRIVER_INSTANCE(Raspi_videocore_mailbox, "videocore", "Raspi Videocore Firmware", driver::Graphics)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto set_mode(U32 framebufferId, U32 width, U32 height, graphics2d::BufferFormat format, bool acceptSuggestion = true) -> Try<> override;

		auto get_mode_count() -> U32 override;
		auto get_mode(U32 framebufferId, U32 index) -> framebuffer::Mode override;
		auto set_mode(U32 framebufferId, U32 index) -> Try<> override;
		
		auto detect_default_mode() -> bool override;
		auto get_default_mode() -> framebuffer::Mode override;

		auto get_framebuffer_count() -> U32 override;
		auto get_framebuffer(U32 index) -> Framebuffer* override;
		auto get_framebuffer_name(U32 index) -> const char* override;
	};
}
