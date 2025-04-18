#include "Gui.hpp"

#include "Control.hpp"

namespace {
	const auto windowBackgroundColour = 0xeeeeee;
}

namespace ui2d {
	/**/ Gui:: Gui(graphics2d::Buffer buffer, Theme &theme):
		buffer(buffer),
		theme(theme),
		backgroundColour(windowBackgroundColour)
	{}

	/**/ Gui::~Gui(){}

	void Gui::on_mouse_left(){
	}
	void Gui::on_mouse_moved(I32 x, I32 y){
		for(auto control:controls) control->on_mouse_moved(x, y);
	}
	void Gui::on_mouse_pressed(I32 x, I32 y, U32 button){
		for(auto control:controls) control->on_mouse_pressed(x, y, button);
	}
	void Gui::on_mouse_released(I32 x, I32 y, U32 button){
		for(auto control:controls) control->on_mouse_released(x, y, button);
	}

	void Gui::redraw(bool flush){
		if(isFrozen) return;

		for(auto control:controls){
			control->redraw(flush);
		}
	}
}
