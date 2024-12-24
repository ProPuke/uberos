#include <kernel/drivers/Graphics.hpp>
#include <kernel/drivers.hpp>

#include <common/graphics2d/BufferFormat.hpp>
#include <common/types.hpp>

namespace driver {
	namespace graphics {
		struct VBE: driver::Graphics {
			typedef driver::Graphics Super;

			/**/ VBE();
			/**/ VBE(U64 address);

			auto _on_start() -> bool override;
			auto _on_stop() -> bool override;

			auto get_mode_count(U32 framebufferId) -> U32 override;
			auto get_mode(U32 framebufferId, U32 index) -> framebuffer::Mode override;
			auto set_mode(U32 framebufferId, U32 index) -> bool override;
			using driver::Graphics::set_mode;
			
			auto get_framebuffer_count() -> U32 override;
			auto get_framebuffer(U32 index) -> Framebuffer* override;
			auto get_framebuffer_name(U32 index) -> const char* override;
		};
	}
}
