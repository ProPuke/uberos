#include "panic.hpp"

#include <drivers/DisplayManager.hpp>

#include <kernel/debugSymbols.hpp>
#include <kernel/drivers.hpp>
#include <kernel/exceptions.hpp>

#include <common/graphics2d/font.hpp>

namespace panic {
	driver::DisplayManager *displayManager = nullptr;
	graphics2d::Buffer *framebuffer = nullptr;
	Physical<graphics2d::Buffer> framebufferPhysical{0x00};

	namespace {
		void on_displayManager_event(const driver::DisplayManager::Event &event) {
			switch(event.type){
				case driver::DisplayManager::Event::Type::framebuffersChanged:
					framebuffer = displayManager->get_screen_count()>0?displayManager->get_screen_buffer(0):nullptr;
					framebufferPhysical.address = mmu::kernel::transaction().get_physical(framebuffer).address;
				break;
			}
		}

		void set_displayManager(driver::DisplayManager *set) {
			if(displayManager==set) return;

			if(displayManager){
				displayManager->events.unsubscribe(on_displayManager_event);
			}

			displayManager = set;
			if(displayManager){
				displayManager->events.subscribe(on_displayManager_event);
				framebuffer = displayManager->get_screen_count()>0?displayManager->get_screen_buffer(0):nullptr;
			}else{
				framebuffer = nullptr;
			}

			framebufferPhysical.address = mmu::kernel::transaction().get_physical(framebuffer).address;
		}

		void on_drivers_event(const drivers::Event &event){
			switch(event.type){
				case drivers::Event::Type::driverInstalled:
				break;
				case drivers::Event::Type::driverStarted:
					if(auto displayManager = event.driverStarted.driver->as_type<driver::DisplayManager>()){
						set_displayManager(displayManager);
					}
				break;
				case drivers::Event::Type::driverStopped:
					if(event.driverStarted.driver==displayManager){
						set_displayManager(drivers::find_active<driver::DisplayManager>());
					}
				break;
			}
		}
	}

	void init() {
		drivers::events.subscribe(on_drivers_event);
	}

	auto panic() -> Panic {
		return panic("Critical Failure", "An unrecoverable error occurred");
	}

	auto panic(const char *title, const char *subtitle) -> Panic {
		return {framebufferPhysical, title, subtitle};
	}

	/**/ Panic:: Panic(Physical<graphics2d::Buffer> framebufferPhysical, const char *title, const char *subtitle):
		framebuffer((graphics2d::Buffer*)framebufferPhysical.address)
	{
		exceptions::disable();

		if(framebuffer){
			width = maths::clamp(700u*framebuffer->height/1080u, 450u, framebuffer->width);
			x = (framebuffer->width-width)/2;
			y = 10u;
			cursorX = x;

			for(auto y=0u; y<framebuffer->height; y++) for(auto x=0u; x<framebuffer->width; x++) {
				// framebuffer->set_blended(x, y, graphics2d::premultiply_colour(0xa0ff0000));
				// framebuffer->set_blended(x, y, graphics2d::premultiply_colour(0x20ffcccc));
				framebuffer->set_blended(x, y, graphics2d::premultiply_colour(0x20cccccc));
			}

			{
				auto thickness = max(4u, 8u*framebuffer->height/1080u);
				auto margin = 40u*framebuffer->height/1080u;
				auto size = max(80u, 150u*framebuffer->height/1080u);
				auto height = (I32)(size * 0.86602540378);

				const auto widthWithIcon = width+size+margin;

				if(widthWithIcon<framebuffer->width){
					x += (widthWithIcon-width)/2;

					auto top = y+(size-height);

					for(auto i=0u;i<thickness;i++){
						framebuffer->draw_line_aa(x-margin-size+i/2, top+height+i, x-margin-i/2, top+height+i, 0x000000);
						framebuffer->draw_line_aa(x-margin-size+i, top+height, x-margin-size/2-thickness/2+i, top, 0x000000);
						framebuffer->draw_line_aa(x-margin-i, top+height, x-margin-size/2+thickness/2-i, top, 0x000000);
					}
					//NOTE:HACK: aa lines don't (didn't) support line width yet, so we get striping when we stack them up in a loop like thus. We cheat and draw the inner lines twice to black this out
					for(auto i=1u;i<thickness-1;i++){
						framebuffer->draw_line(x-margin-size+i/2, top+height+i, x-margin-i/2, top+height+i, 0x000000);
						framebuffer->draw_line(x-margin-size+i, top+height, x-margin-size/2-thickness/2+i, top, 0x000000);
						framebuffer->draw_line(x-margin-i, top+height, x-margin-size/2+thickness/2-i, top, 0x000000);
					}

					framebuffer->draw_rect({
						(I32)(x-margin-size/2-thickness/2), (I32)(top+height-thickness-thickness),
						(I32)(x-margin-size/2+thickness/2), (I32)(top+height-thickness)
					}, 0x000000);

					framebuffer->draw_rect({
						(I32)(x-margin-size/2-thickness/2), (I32)(top+thickness+thickness+thickness+thickness),
						(I32)(x-margin-size/2+thickness/2), (I32)(top+height-thickness-thickness-thickness-thickness)
					}, 0x000000);
				}
			}

			const auto titleSize = max(56u, 92u*framebuffer->height/1080u);
			const auto subtitleSize = max(16u, 30u*framebuffer->height/1080u);

			y += titleSize;
			y = framebuffer->draw_text({.font=*graphics2d::font::default_sans_bold, .size=titleSize}, title, x, y, width, 0x000000).y;

			y += 36u*framebuffer->height/1080u;

			y += subtitleSize;
			y = framebuffer->draw_text({.font=*graphics2d::font::default_sans, .size=subtitleSize, .charSpacing=2*(I32)framebuffer->height/1080}, subtitle, x, y, width, 0x000000).y;
			
			y += 50u*framebuffer->height/1080u;
		}
	}

	namespace {
		auto detailsFontSize = 12u;
	}

	/**/ Panic::~Panic() {
		if(!framebuffer&&!hasDetails){
			logging::print_error("An unrecoverable error occurred");
		}
		
		halt();
	}

	auto Panic::print_details_start() -> Panic& {
		if(framebuffer){
			y += (U32)(graphics2d::font::default_console->lineHeight * max(10u, detailsFontSize*framebuffer->height/1080u) + 0.5);
			cursorX = x;

		}else{
			logging::print_error_start();
		}

		return *this;
	}

	auto Panic::print_details_end() -> Panic& {
		if(framebuffer){
			cursorX = x;

		}else{
			logging::print_end();
		}

		return *this;
	}

	auto Panic::_print_details_inline(const char *text) -> Panic& {
		if(framebuffer){
			auto result = framebuffer->draw_text({.font=*graphics2d::font::default_console, .size=max(10u, detailsFontSize*framebuffer->height/1080u)}, text, x, y, width, 0x000000, cursorX);
			cursorX = result.x;
			y = result.y;

		}else{
			logging::print_inline(text);
		}

		hasDetails = true;

		return *this;
	}
}
