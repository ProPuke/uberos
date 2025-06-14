#include "ApplicationWindow.hpp"

#include <drivers/ThemeManager.hpp>

#include <kernel/drivers.hpp>

namespace ui2d {
	namespace {
		AutomaticDriverReference<driver::DesktopManager> desktopManager;
		AutomaticDriverReference<driver::ThemeManager> themeManager;
	}

	/**/ ApplicationWindow::ApplicationWindow(const char *title, I32 width, I32 height):
		desktopWindow(assert(desktopManager.get())->create_standard_window(title, width, height)),
		guiLayout(gui, controlContainer::Box::Direction::vertical)
	{
		const auto clientArea = desktopWindow.get_client_area();
		guiLayout.set_rect({0, 0, clientArea.width(), clientArea.height()});
		guiLayout.set_style(controlContainer::Box::Style::padded);

		desktopWindow.events.subscribe([](const driver::DesktopManager::Window::Event &event, void *_self){
			auto &self = *(ApplicationWindow*)_self;

			if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
				self.gui.buffer = self.desktopWindow.get_client_buffer();
				const auto clientArea = self.desktopWindow.get_client_area();
				self.guiLayout.set_rect({0, 0, clientArea.width(), clientArea.height()});
				self.redraw();

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseLeft){
				self.gui.on_mouse_left();

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseMoved){
				self.gui.on_mouse_moved(event.mouseMoved.x, event.mouseMoved.y);

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mousePressed){
				self.gui.on_mouse_pressed(event.mousePressed.x, event.mousePressed.y, event.mousePressed.button);

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseReleased){
				self.gui.on_mouse_released(event.mouseReleased.x, event.mouseReleased.y, event.mouseReleased.button);
			}
		}, this);
	}

	/**/ ApplicationWindow::Gui::Gui():
		Super(window().desktopWindow.get_client_buffer(), &assert(themeManager.get())->get_theme())
	{}

	void ApplicationWindow::Gui::update_area(graphics2d::Rect rect) {
		window().desktopWindow.redraw_area(rect);
	}

	void ApplicationWindow::redraw() {
		gui.theme = &assert(themeManager.get())->get_theme();

		const auto padding = 0;

		auto minSize = guiLayout.get_min_size();
		auto maxSize = guiLayout.get_max_size();

		desktopWindow.set_client_size_limits(
			minSize.x + padding*2u, minSize.y + padding*2u,
			maxSize.x + padding*2u, maxSize.y + padding*2u
		);

		gui.redraw();
	}

	void ApplicationWindow::layout(bool redraw) {
		auto area = guiLayout.rect;
		if(redraw){
			gui.buffer.draw_rect(guiLayout.rect, gui.backgroundColour);
		}

		guiLayout.layout();

		if(redraw){
			area = area.include(guiLayout.rect);
			gui.redraw();
			gui.update_area(area);
		}
	}

	void ApplicationWindow::show() {
		desktopWindow.show();
	}

	void ApplicationWindow::hide() {
		desktopWindow.hide();
	}
}
