#include "fontTest.hpp"

#include <drivers/DesktopManager.hpp>

#include <kernel/drivers.hpp>

#include <common/graphics2d/font.hpp>

namespace tests::fontTest {
	namespace {
		driver::DesktopManager *desktopManager;
		driver::DesktopManager::StandardWindow *window;
	}

	void run() {
		desktopManager = drivers::find_and_activate<driver::DesktopManager>();
		if(!desktopManager) return;

		window = &desktopManager->create_standard_window("Font Test", 320, 320);
		// view = graphics2d::create_view(nullptr, graphics2d::DisplayLayer::topMost, margin, margin, min(1300u, framebuffer.buffer.width-margin*2), 256);

		// static auto scale = 30u;
		static auto scale = 1u;

		static auto redraw = [](){
			auto &clientArea = window->get_client_area();

			clientArea.draw_rect(0, 0, window->get_width(), window->get_height(), window->get_background_colour());

			auto sizeFontSettings = graphics2d::Buffer::FontSettings{
				.font = *graphics2d::font::default_sans,
				.size = 10,
				.maxLines = 1
			};

			auto sizeWidth = clientArea.measure_text(sizeFontSettings, "888 ").blockWidth;

			auto fontSettings = graphics2d::Buffer::FontSettings{
				.font = *graphics2d::font::default_sans,
				.size = scale,
				.maxLines = 1
			};

			for(auto y=max(sizeFontSettings.size, fontSettings.size);y<(U32)window->get_height();y++){
				// if(fontSettings.size>24){
				// 	clientArea.draw_text(fontSettings, "Abc", 10+1, y+1, clientArea.width, 0x222222);
				// 	clientArea.draw_text(fontSettings, "Abc", 10, y, clientArea.width, 0xaaaaaa);
				// }else{
					clientArea.draw_text(sizeFontSettings, to_string(fontSettings.size), 10, y, clientArea.width, 0x444444);
					clientArea.draw_text(fontSettings, "Abc", 10+sizeWidth, y, clientArea.width, 0x444444);
				// }

				fontSettings.size += scale;
				y += max(sizeFontSettings.size, fontSettings.size) * 4 / 5;
			}

			static char statusBuffer[64];
			strcpy(statusBuffer, "Step: ");
			strcat(statusBuffer, utoa(scale));
			window->set_status(statusBuffer);

			window->redraw();
		};

		redraw();
		window->show();

		window->events.subscribe([](const driver::DesktopManager::Window::Event &event, void*){
			if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
				redraw();

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseScrolled){
				scale = maths::clamp((I32)scale - event.mouseScrolled.distance*(scale>20?3:scale>16?2:1), 1, 60);
				redraw();
			}

		}, nullptr);
	}
}
