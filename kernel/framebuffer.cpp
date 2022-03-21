#include "framebuffer.hpp"

#include <kernel/driver/Graphics.hpp>
#include <kernel/deviceManager.hpp>

namespace framebuffer {
	void init() {
		stdio::Section section("framebuffer::init...");

		{
			stdio::Section section("Devices:");

			for(auto device=deviceManager::devices.head; device; device=device->next) {
				if(!strcmp(device->type, "graphics")){
					auto &graphics = *(driver::Graphics*)device;

					stdio::Section section(graphics.name);

					{
						stdio::Section section("Possible video modes:");

						U32 count = 0;
						for(U32 i=0;i<graphics.get_mode_count();i++){
							auto mode = graphics.get_mode(0, i);

							if(!mode.width) continue;

							stdio::print_info(mode.width, "x", mode.height, " @ ", mode.format);
							count++;
						}

						if(!count){
							stdio::print_warning("None found");
						}
					}
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

		for(auto device = deviceManager::devices.head; device; device=device->next){
			if(!strcmp(device->type, "graphics")){
				auto graphics = (driver::Graphics*)device;

				count += graphics->get_framebuffer_count();
			}
		}

		return count;
	}

	Framebuffer* get_framebuffer(U32 index) {
		for(auto device = deviceManager::devices.head; device; device=device->next){
			if(!strcmp(device->type, "graphics")){
				auto graphics = (driver::Graphics*)device;

				const auto count = graphics->get_framebuffer_count();
				if(index<count){
					return graphics->get_framebuffer(index);
				}

				index -= count;
			}
		}

		return nullptr;
	}
}
