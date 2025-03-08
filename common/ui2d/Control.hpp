#pragma once

#include "Gui.hpp"

#include <common/graphics2d/Rect.hpp>

namespace ui2d {
	struct Control {
		/**/ Control(Gui &gui, graphics2d::Rect rect):
			gui(gui),
			rect(rect)
		{
			gui.controls.push_back(this);
		}

		virtual /**/~Control(){
			for(auto i=0u;i<gui.controls.length;i++){
				if(gui.controls[i]==this){
					gui.controls.remove(i);
					break;
				}
			}
		}

		Gui &gui;
		graphics2d::Rect rect;
		bool isHover = false;
		bool isPressed = false;

		virtual void on_mouse_moved(I32 x, I32 y) {
			auto hover = rect.contains(x, y);

			if(isHover!=hover){
				isHover = hover;
				redraw();
			}
		}

		virtual void on_mouse_pressed(I32 x, I32 y, U32 button) {
			auto hover = rect.contains(x, y);

			auto pressed = hover&&button==0;

			if(isPressed!=pressed){
				isPressed = pressed;
				redraw();
			}
		}

		virtual void on_mouse_released(I32 x, I32 y, U32 button) {
			if(button==0){
				auto pressed = false;

				if(isPressed!=pressed){
					isPressed = pressed;
					redraw();
				}
			}
		}

		virtual void redraw(bool flush = true) = 0;
	};
}
