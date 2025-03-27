#include "memoryTest.hpp"

#include <drivers/DesktopManager.hpp>

#include <kernel/drivers.hpp>
#include <kernel/PhysicalPointer.hpp>

#include <common/graphics2d/font.hpp>

namespace tests::memoryTest {
	namespace {
		driver::DesktopManager *desktopManager;
		driver::DesktopManager::StandardWindow *window;
	}

	void run() {
		desktopManager = drivers::find_and_activate<driver::DesktopManager>();
		if(!desktopManager) return;

		window = &desktopManager->create_standard_window("Memory Status", 800, 900);

		static auto redraw = [](){
			auto &clientArea = window->get_client_area();

			clientArea.draw_rect(0, 0, window->get_width(), window->get_height(), window->get_background_colour());

			auto fontSettings = graphics2d::Buffer::FontSettings{
				.font = *graphics2d::font::default_sans,
				.size = 14
			};

			auto margin = 10;

			auto x = margin;
			auto y = (I32)(/*margin+*/fontSettings.font.lineHeight*(fontSettings.size+0.5));

			auto pos = clientArea.draw_text(fontSettings, "Total memory: ", x, y, clientArea.width-margin*2, 0x222222);
			pos = clientArea.draw_text(fontSettings, to_string(memory::totalMemory/1024/1024), x, pos.y, clientArea.width-margin*2, 0x222222, pos.x);
			pos = clientArea.draw_text(fontSettings, "MiB\n", x, pos.y, clientArea.width-margin*2, 0x222222, pos.x);
			pos = clientArea.draw_text(fontSettings, "Reserved kernel memory: ", x, pos.y, clientArea.width-margin*2, 0x222222, pos.x);
			pos = clientArea.draw_text(fontSettings, to_string((UPtr)memory::heap.address/1024), x, pos.y, clientArea.width-margin*2, 0x222222, pos.x);
			pos = clientArea.draw_text(fontSettings, "KiB\n", x, pos.y, clientArea.width-margin*2, 0x222222, pos.x);
			pos = clientArea.draw_text(fontSettings, "Heap in use: ", x, pos.y, clientArea.width-margin*2, 0x222222, pos.x);
			pos = clientArea.draw_text(fontSettings, to_string((UPtr)memory::get_used_heap()/1024), x, pos.y, clientArea.width-margin*2, 0x222222, pos.x);
			pos = clientArea.draw_text(fontSettings, "KiB\n", x, pos.y, clientArea.width-margin*2, 0x222222, pos.x);

			y = pos.y - fontSettings.font.lineHeight*(fontSettings.size+0.5) - fontSettings.font.descender*(fontSettings.size+0.5) + margin;

			{
				auto cellSize = 5;
				auto cellSpacing = 0;
				auto cols = (clientArea.width-margin*2+cellSpacing)/(cellSize+cellSpacing);
				U32 corner[2];
				graphics2d::create_diagonal_corner(1, corner);

				for(auto row=0u;;row++){
					for(auto col=0u;col<cols;col++){
						x = margin+col*(cellSize+cellSpacing);

						auto fillColour = 0xffffff;
						auto borderColour = 0xaaaaaa;

						auto page = row*cols+col;
						auto address = (page*memory::pageSize);
						if(address>=(memory::heap+memory::heapSize).address) goto allMemoryDone;

						// if(address<memory::stack.address){
						// 	// kernel code memory
						// 	fillColour = 0xff0000;
						// }else if(address<memory::heap.address){
						// 	// kernel stack memory
						// 	fillColour = 0x00ff00;
						// }else{
						// 	// heap memory
						// 	fillColour = 0x0000ff;
						// }

						auto optionsResponse = mmu::kernel::transaction().get_virtual_options((void*)address);
						if(!optionsResponse){
							borderColour = 0xdddddd;
						}else{
							auto options = optionsResponse.result;

							switch(options.caching){
								case mmu::Caching::uncached:
									fillColour = 0xaa2222;
									borderColour = 0xff0000;
								break;
								case mmu::Caching::writeBack:
									fillColour = 0x00aa00;
									borderColour = 0x000000;
								break;
								case mmu::Caching::writeCombining:
									fillColour = 0x0060aa;
									borderColour = 0x000000;
								break;
								case mmu::Caching::writeThrough:
									fillColour = 0x00aaaa;
									borderColour = 0x000000;
								break;
							}

							if(options.isWritable){
								borderColour = window->get_background_colour();
							}
						}

						clientArea.draw_rect({x, y, x+cellSize, y+cellSize}, fillColour, corner, corner, corner, corner);
						clientArea.draw_rect_outline({x, y, x+cellSize, y+cellSize}, borderColour, 1, corner, corner, corner, corner);
					}
					y += cellSize+cellSpacing;
				}

				allMemoryDone: ;
			}

			window->redraw();
		};

		redraw();
		window->show();

		window->events.subscribe([](const driver::DesktopManager::Window::Event &event, void*){
			if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
				redraw();
			}

		}, nullptr);
	}
}
