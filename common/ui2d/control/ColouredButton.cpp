#include "ColouredButton.hpp"

namespace ui2d {
	namespace control {
		/**/ ColouredButton::ColouredButton(Gui &gui, graphics2d::Rect rect, U32 colour, const char *text):
			Button(gui, rect, text),
			colour(colour)
		{}

		void ColouredButton::redraw(bool flush) {
			switch(type){
				case Type::regular:
					gui.theme.draw_coloured_button(gui.buffer, rect, gui.backgroundColour, colour, opacity, text, icon, isHover, isHover&&isPressed);
				break;
				case Type::toggle:
					gui.theme.draw_coloured_toggle_button(gui.buffer, rect, gui.backgroundColour, colour, opacity, text, icon, toggleActive||(isHover&&isPressed), isHover, isHover&&isPressed);
				break;
			}
			if(flush){
				gui.update_area(rect);
			}
		}
	}
}
