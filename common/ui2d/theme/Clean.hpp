#pragma once

#include "../Theme.hpp"

namespace ui2d {
	namespace theme {
		struct Clean: Theme {
			auto get_component_spacing() -> U32 override;
			auto get_section_spacing() -> U32 override;
	
			auto get_minimum_button_width() -> U32 override;
			auto get_minimum_button_height() -> U32 override;
			void draw_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, bool smallFont, graphics2d::Buffer *icon, bool isHover, bool isDown) override;
			void draw_coloured_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon, bool isHover = false, bool isDown = false) override;
			void draw_toggle_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, bool smallFont, graphics2d::Buffer *icon, bool toggleState, bool isHover, bool isDown) override;
			void draw_coloured_toggle_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon, bool toggleState, bool isHover, bool isDown) override;
		};
	}
}