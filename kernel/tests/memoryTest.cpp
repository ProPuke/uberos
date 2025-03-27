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

		window = &desktopManager->create_standard_window("Memory Status", 700, 600);

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

			{
				const auto width = clientArea.width-margin;
				auto leftPos = clientArea.draw_text(fontSettings, "Total memory: ", x, y, width, 0x222222);
				leftPos = clientArea.draw_text(fontSettings, to_string(memory::totalMemory/1024/1024), x, leftPos.y, width, 0x222222, leftPos.x);
				leftPos = clientArea.draw_text(fontSettings, "MiB\n", x, leftPos.y, width, 0x222222, leftPos.x);
				leftPos = clientArea.draw_text(fontSettings, "Total kernel size: ", x, leftPos.y, width, 0x222222, leftPos.x);
				leftPos = clientArea.draw_text(fontSettings, to_string((UPtr)(memory::heap.address-memory::code.address)/1024), x, leftPos.y, width, 0x222222, leftPos.x);
				leftPos = clientArea.draw_text(fontSettings, "KiB\n", x, leftPos.y, width, 0x222222, leftPos.x);
				leftPos = clientArea.draw_text(fontSettings, "Heap in use: ", x, leftPos.y, width, 0x222222, leftPos.x);
				leftPos = clientArea.draw_text(fontSettings, to_string((UPtr)memory::get_used_heap()/1024), x, leftPos.y, width, 0x222222, leftPos.x);
				leftPos = clientArea.draw_text(fontSettings, "KiB\n", x, leftPos.y, width, 0x222222, leftPos.x);

				x = clientArea.width/2+margin;
				auto rightPos = clientArea.draw_text(fontSettings, "Reserved low memory: ", x, y, width, 0x222222);
				rightPos = clientArea.draw_text(fontSettings, to_string((UPtr)(memory::code.address)/1024), x, rightPos.y, width, 0x222222, rightPos.x);
				rightPos = clientArea.draw_text(fontSettings, "KiB\n", x, rightPos.y, width, 0x222222, rightPos.x);
				rightPos = clientArea.draw_text(fontSettings, "Kernel code size: ", x, rightPos.y, width, 0x222222, rightPos.x);
				rightPos = clientArea.draw_text(fontSettings, to_string((UPtr)memory::codeSize/1024), x, rightPos.y, width, 0x222222, rightPos.x);
				rightPos = clientArea.draw_text(fontSettings, "KiB\n", x, rightPos.y, width, 0x222222, rightPos.x);
				rightPos = clientArea.draw_text(fontSettings, "Kernel data size: ", x, rightPos.y, width, 0x222222, rightPos.x);
				rightPos = clientArea.draw_text(fontSettings, to_string((UPtr)(memory::constantsSize+memory::initialisedDataSize+memory::uninitialisedDataSize)/1024), x, rightPos.y, width, 0x222222, rightPos.x);
				rightPos = clientArea.draw_text(fontSettings, "KiB\n", x, rightPos.y, width, 0x222222, rightPos.x);

				y = max(leftPos.y, rightPos.y) - fontSettings.font.lineHeight*(fontSettings.size+0.5) - fontSettings.font.descender*(fontSettings.size+0.5) + margin;
			}


			{
				auto cellSize = 5;
				auto cellSpacing = 0;
				auto cols = (clientArea.width-margin*2+cellSpacing)/(cellSize+cellSpacing);
				U32 corner[2];
				graphics2d::create_diagonal_corner(1, corner);

				auto upperMemoryBottom = 0;

				for(auto row=0u;;row++){
					for(auto col=0u;col<cols;col++){
						x = margin+col*(cellSize+cellSpacing);

						auto fillColour = 0xffffff;
						auto borderColour = 0xaaaaaa;

						auto page = row*cols+col;
						auto address = (page*memory::pageSize);
						if(address>=(memory::stack+memory::stackSize).address) goto lowerMemoryDone;

						auto optionsResponse = mmu::kernel::transaction().get_virtual_options((void*)address);
						if(!optionsResponse){
							borderColour = 0xdddddd;
						}else{
							auto options = optionsResponse.result;

							switch(options.caching){
								case mmu::Caching::uncached:
									fillColour = 0xaa2222;
								break;
								case mmu::Caching::writeBack:
									fillColour = 0x00aa00;
								break;
								case mmu::Caching::writeCombining:
									fillColour = 0x00aaaa;
								break;
								case mmu::Caching::writeThrough:
									fillColour = 0x0060aa;
								break;
							}

							borderColour = fillColour;

							if(options.isWritable){
								borderColour = graphics2d::blend_colours(0x222222, graphics2d::premultiply_colour(borderColour|(0x33<<24)));
							}else{
								borderColour = window->get_background_colour();
							}
						}

						clientArea.draw_rect({x, y, x+cellSize, y+cellSize}, fillColour, corner, corner, corner, corner);
						clientArea.draw_rect_outline({x, y, x+cellSize, y+cellSize}, borderColour, 1, corner, corner, corner, corner);
					}
					y += cellSize+cellSpacing;
				}

				lowerMemoryDone:

				upperMemoryBottom = y;

				y = clientArea.height-margin-cellSize;

				for(auto row=0u;;row++){
					if(y<=upperMemoryBottom+margin) goto upperMemoryDone;

					for(auto col=0u;col<cols;col++){
						x = margin+col*(cellSize+cellSpacing);

						auto fillColour = 0xffffff;
						auto borderColour = 0xaaaaaa;

						auto page = (0xffffffffffffffff)/memory::pageSize - cols + col - row*cols;
						auto address = (page*memory::pageSize);
						if(address<=(memory::stack+memory::stackSize).address) goto upperMemoryDone;

						auto optionsResponse = mmu::kernel::transaction().get_virtual_options((void*)address);
						if(!optionsResponse){
							borderColour = 0xdddddd;
						}else{
							auto options = optionsResponse.result;

							switch(options.caching){
								case mmu::Caching::uncached:
									fillColour = 0xaa2222;
								break;
								case mmu::Caching::writeBack:
									fillColour = 0x00aa00;
								break;
								case mmu::Caching::writeCombining:
									fillColour = 0x00aaaa;
								break;
								case mmu::Caching::writeThrough:
									fillColour = 0x0060aa;
								break;
							}

							borderColour = fillColour;

							if(options.isWritable){
								borderColour = graphics2d::blend_colours(0x222222, graphics2d::premultiply_colour(borderColour|(0x33<<24)));
							}else{
								borderColour = window->get_background_colour();
							}
						}

						clientArea.draw_rect({x, y, x+cellSize, y+cellSize}, fillColour, corner, corner, corner, corner);
						clientArea.draw_rect_outline({x, y, x+cellSize, y+cellSize}, borderColour, 1, corner, corner, corner, corner);
					}
					y -= cellSize+cellSpacing;
				}

				upperMemoryDone: ;
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
