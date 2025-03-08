#pragma once

#include <common/graphics2d/Buffer.hpp>

namespace ui2d {
	struct Theme {
		/*   */ /**/ Theme(){}
		virtual /**/~Theme(){}

		virtual auto get_component_spacing() -> U32 = 0;
		virtual auto get_section_spacing() -> U32 = 0;

		virtual auto get_minimum_button_width() -> U32 = 0;
		virtual auto get_minimum_button_height() -> U32 = 0;
		virtual void draw_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, graphics2d::Buffer *icon = nullptr, bool isHover = false, bool isDown = false) = 0;
		virtual void draw_coloured_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, graphics2d::Buffer *icon = nullptr, bool isHover = false, bool isDown = false) = 0;
		virtual void draw_toggle_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, graphics2d::Buffer *icon, bool toggleState, bool isHover = false, bool isDown = false) = 0;
		virtual void draw_coloured_toggle_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, graphics2d::Buffer *icon, bool toggleState, bool isHover = false, bool isDown = false) = 0;
	};
}
