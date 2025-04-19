#pragma once

#include "../Theme.hpp"

namespace ui2d {
	namespace theme {
		struct Clean: Theme {
			/**/ Clean();

			auto get_component_spacing() -> U32 override;
			auto get_section_spacing() -> U32 override;
	
			auto get_minimum_button_width() -> U32 override;
			auto get_minimum_button_height() -> U32 override;
			void draw_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, bool smallFont, graphics2d::Buffer *icon, bool isHover, bool isDown) override;
			void draw_coloured_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon, bool isHover = false, bool isDown = false) override;
			void draw_toggle_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, bool smallFont, graphics2d::Buffer *icon, bool toggleState, bool isHover, bool isDown) override;
			void draw_coloured_toggle_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon, bool toggleState, bool isHover, bool isDown) override;

			auto get_window_left_margin() -> U32 override;
			auto get_window_right_margin() -> U32 override;
			auto get_window_top_margin() -> U32 override;
			auto get_window_bottom_margin() -> U32 override;
			auto get_window_min_width() -> U32 override;
			auto get_window_min_height() -> U32 override;
			auto get_window_titlebar_area(graphics2d::Rect) -> graphics2d::Rect override;
			auto get_window_titlebar_divider_colour(WindowFrameOptions) -> U32 override;
			auto get_window_client_area(graphics2d::Rect) -> graphics2d::Rect override;
			auto get_window_solid_area(graphics2d::Rect) -> graphics2d::Rect override;
			auto get_window_interact_area(graphics2d::Rect) -> graphics2d::Rect override;
			void draw_window_frame(graphics2d::Buffer&, graphics2d::Rect, WindowFrameOptions options) override;
		};
	}
}