#pragma once

#include <drivers/DesktopManager.hpp>

#include <common/ui2d/controlContainer/Box.hpp>
#include <common/ui2d/Gui.hpp>

namespace ui2d {
	struct ApplicationWindow {
		/**/ ApplicationWindow(const char *title, I32 width, I32 height);

		driver::DesktopManager::Window &desktopWindow;

		void layout(bool redraw = true);
		void redraw();

		struct Gui: ui2d::Gui {
			typedef ui2d::Gui Super;

			/**/ Gui();

			void update_area(graphics2d::Rect) override;

			auto window() -> ApplicationWindow& { return *(ApplicationWindow*)((U8*)(this)-offsetof(ApplicationWindow, gui)); }
		} gui;

		ui2d::controlContainer::Box guiLayout;

		void show();
		void hide();
	};
}
