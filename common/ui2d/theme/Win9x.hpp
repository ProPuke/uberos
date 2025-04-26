#pragma once

#include <common/ui2d/Theme.hpp>

namespace ui2d {
	namespace theme {
		struct Win9x: Theme {
			/**/ Win9x();

			auto get_component_spacing() -> U32 override;
			auto get_section_spacing() -> U32 override;
	
			auto get_minimum_button_width() -> U32 override;
			auto get_minimum_button_height() -> U32 override;
			void draw_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, bool smallFont, graphics2d::Buffer *icon, bool isHover, bool isDown) override;
			void draw_coloured_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon, bool isHover = false, bool isDown = false) override;
			void draw_toggle_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, bool smallFont, graphics2d::Buffer *icon, bool toggleState, bool isHover, bool isDown) override;
			void draw_coloured_toggle_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon, bool toggleState, bool isHover, bool isDown) override;

			auto get_window_min_width() -> U32 override;
			auto get_window_min_height() -> U32 override;
			auto get_window_titlebar_area(graphics2d::Rect) -> graphics2d::Rect override;
			auto get_window_titlebar_divider_colour(WindowFrameOptions) -> U32 override;
			auto get_window_titlebar_widget_margin() -> U32 override;
			auto get_window_titlebar_widget_spacing() -> U32 override;
			auto get_window_titlebar_widget_large_spacing() -> U32 override;
			auto has_window_titlebar_widget_coloured() -> bool override;
			auto get_window_titlebar_widget_minimise() -> graphics2d::Buffer& override;
			auto get_window_titlebar_widget_maximise() -> graphics2d::Buffer& override;
			auto get_window_titlebar_widget_close() -> graphics2d::Buffer& override;
			auto get_window_titlebar_widget_restore() -> graphics2d::Buffer& override;
			auto get_window_statusbar_area(graphics2d::Rect) -> graphics2d::Rect override;
			auto get_window_client_area(graphics2d::Rect) -> graphics2d::Rect override;
			auto get_window_solid_area(graphics2d::Rect) -> graphics2d::Rect override;
			auto get_window_interact_area(graphics2d::Rect) -> graphics2d::Rect override;
			auto get_window_background_colour() -> U32 override;
			void draw_window_frame(graphics2d::Buffer&, graphics2d::Rect, WindowFrameOptions options) override;
		};
	}
}