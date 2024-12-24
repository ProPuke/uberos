#include "framebuffer.hpp"

#include <kernel/drivers.hpp>
#include <kernel/drivers/Graphics.hpp>
#include <kernel/Log.hpp>

static Log log("framebuffer");

namespace framebuffer {
	void init() {
		auto section = log.section("init...");

		{
			auto section = log.section("Devices:");

			for(auto &graphics:drivers::iterate<driver::Graphics>()){
				log.print_info(graphics.name, ':');
				for(auto i=0u,framebuffers=graphics.get_framebuffer_count();i<framebuffers;i++){
					log.print_info("  ", graphics.get_framebuffer_name(i));
				}
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

		for(auto &graphics:drivers::iterate<driver::Graphics>()){
			count += graphics.get_framebuffer_count();
		}

		return count;
	}

	Framebuffer* get_framebuffer(U32 index) {
		for(auto &graphics:drivers::iterate<driver::Graphics>()){
			const auto count = graphics.get_framebuffer_count();
			if(index<count){
				return graphics.get_framebuffer(index);
			}

			index -= count;
		}

		return nullptr;
	}
}
