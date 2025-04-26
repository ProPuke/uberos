#include "TitlebarButton.hpp"

namespace ui2d {
	namespace control {
		/**/ TitlebarButton::TitlebarButton(Gui &gui, graphics2d::Rect rect, U32 colour, const char *text):
			ColouredButton(gui, rect, colour, text)
		{}

		void TitlebarButton::redraw(bool flush) {
			if(!isVisible) return;

			switch(type){
				case Type::regular:
					gui.theme.draw_titlebar_button(gui.buffer, rect, gui.backgroundColour, colour, opacity, text, smallFont, icon, isHover, isHover&&isPressed);
				break;
				case Type::toggle:
					gui.theme.draw_titlebar_toggle_button(gui.buffer, rect, gui.backgroundColour, colour, opacity, text, smallFont, icon, toggleActive||(isHover&&isPressed), isHover, isHover&&isPressed);
				break;
			}

			if(flush){
				gui.update_area(rect);
			}
		}
	}
}
