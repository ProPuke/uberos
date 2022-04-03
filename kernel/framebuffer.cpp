#include "framebuffer.hpp"

#include <kernel/driver/Graphics.hpp>
#include <kernel/device.hpp>

namespace framebuffer {
	void init() {
		stdio::Section section("framebuffer::init...");

		{
			stdio::Section section("Devices:");

			for(auto &graphics:device::iterate_type<driver::Graphics>("graphics")){
				stdio::print_info(graphics.name);
			}
		}

		// {
		// 	auto &framebuffer = framebuffers[0];
		// 	auto buffer = graphics2d::get_screen_buffer(0, {0, 0, (I32)framebuffer.width, (I32)framebuffer.height});
		// 	while(true){
		// 		graphics2d::update_background();
		// 		for(U32 x=0;x<framebuffer.width/4;x++){
		// 			buffer.scroll(-4, -2);
		// 		}
		// 	}
		// }
	}

	U32 get_framebuffer_count() {
		U32 count = 0;

		for(auto &graphics:device::iterate_type<driver::Graphics>("graphics")){
			count += graphics.get_framebuffer_count();
		}

		return count;
	}

	Framebuffer* get_framebuffer(U32 index) {
		for(auto &graphics:device::iterate_type<driver::Graphics>("graphics")){
			const auto count = graphics.get_framebuffer_count();
			if(index<count){
				return graphics.get_framebuffer(index);
			}

			index -= count;
		}

		return nullptr;
	}
}
