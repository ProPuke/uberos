#include "Button.hpp"

#include <kernel/logging.hpp>

namespace ui2d {
	namespace control {
		void Button::redraw(bool flush) {
			if(!isVisible) return;

			switch(type){
				case Type::regular:
					gui.theme.draw_button(gui.buffer, rect, text, smallFont, icon, isHover, isHover&&isPressed);
				break;
				case Type::toggle:
					gui.theme.draw_toggle_button(gui.buffer, rect, text, smallFont, icon, toggleActive||(isHover&&isPressed), isHover, isHover&&isPressed);
				break;
			}

			if(flush){
				gui.update_area(rect);
			}
		}

		void Button::on_mouse_pressed(I32 x, I32 y, U32 button) {
			Super::on_mouse_pressed(x, y, button);

			if(button==0&&rect.contains(x, y)){
				on_pressed();
			}
		}

		void Button::on_mouse_released(I32 x, I32 y, U32 button) {
			auto wasClicked = false;
			auto wasReleased = false;

			if(button==0&&isPressed){
				wasReleased = true;
			}

			if(rect.contains(x, y)){
				if(isPressed){
					switch(type){
						case Type::regular:
						break;
						case Type::toggle:
							toggleActive = !toggleActive;
						break;
					}
					wasClicked = true;
				}
			}

			Super::on_mouse_released(x, y, button);

			if(wasReleased){
				on_released();
			}

			if(wasClicked){
				on_clicked();
			}
		}
	}
}
