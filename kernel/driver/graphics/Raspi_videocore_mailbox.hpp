#include <common/types.hpp>

#include <kernel/driver/Graphics.hpp>
#include <kernel/device.hpp>

namespace driver {
	namespace graphics {
		struct Raspi_videocore_mailbox final: driver::Graphics {
			/**/ Raspi_videocore_mailbox(U64 address);

			void _on_driver_enable() override;
			void _on_driver_disable() override;

			auto set_mode(U32 framebufferId, U32 width, U32 height, FramebufferFormat format, bool acceptSuggestion = true) -> bool override;

			auto get_mode_count() -> U32 override;
			auto get_mode(U32 framebufferId, U32 index) -> framebuffer::Mode override;
			
			auto detect_default_mode() -> bool override;
			auto get_default_mode() -> framebuffer::Mode override;

			auto get_framebuffer_count() -> U32 override;
			auto get_framebuffer(U32 index) -> Framebuffer* override;
		};
	}
}
