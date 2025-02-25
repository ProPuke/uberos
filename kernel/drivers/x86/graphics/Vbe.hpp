#include <kernel/drivers/Graphics.hpp>
#include <kernel/drivers.hpp>

#include <common/graphics2d/BufferFormat.hpp>
#include <common/Try.hpp>
#include <common/types.hpp>

namespace driver::graphics {
	struct Vbe final: driver::Graphics {
		DRIVER_INSTANCE(Vbe, 0x9695f10e, "vbe", "VESA BIOS Extensions", driver::Graphics)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto get_mode_count(U32 framebufferId) -> U32 override;
		auto get_mode(U32 framebufferId, U32 index) -> Mode override;
		auto set_mode(U32 framebufferId, U32 index) -> Try<> override;
		using driver::Graphics::set_mode;
		
		auto get_framebuffer_count() -> U32 override;
		auto get_framebuffer(U32 index) -> graphics2d::Buffer* override;
		auto get_framebuffer_name(U32 index) -> const char* override;
	};
}
