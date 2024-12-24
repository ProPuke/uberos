#include <kernel/drivers/Graphics.hpp>
#include <kernel/drivers.hpp>

#include <common/graphics2d/BufferFormat.hpp>
#include <common/types.hpp>

namespace driver {
	namespace graphics {
		struct Raspi_videocore_mailbox final: driver::Graphics {
			typedef driver::Graphics Super;

			/**/ Raspi_videocore_mailbox(U64 address);

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;

			auto set_mode(U32 framebufferId, U32 width, U32 height, graphics2d::BufferFormat format, bool acceptSuggestion = true) -> bool override;

			auto get_mode_count() -> U32 override;
			auto get_mode(U32 framebufferId, U32 index) -> framebuffer::Mode override;
			auto set_mode(U32 framebufferId, U32 index) -> bool override;
			
			auto detect_default_mode() -> bool override;
			auto get_default_mode() -> framebuffer::Mode override;

			auto get_framebuffer_count() -> U32 override;
			auto get_framebuffer(U32 index) -> Framebuffer* override;
			auto get_framebuffer_name(U32 index) -> const char* override;
		};
	}
}
