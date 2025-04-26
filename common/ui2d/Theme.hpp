#pragma once

#include <common/graphics2d/Buffer.hpp>

namespace ui2d {
	struct Theme {
		/*   */ /**/ Theme();
		virtual /**/~Theme(){}

		U32 leftShadow = 0;
		U32 rightShadow = 0;
		U32 topShadow = 0;
		U32 bottomShadow = 0;
	
		U8 shadowIntensity = 20; // max intensity
		U8 topShadowIntensity = 128; // scaling down of top shadow (by inner extension)
		U8 leftShadowIntensity = 192; // scaling down of left shadow (by inner extension)
		U8 rightShadowIntensity = 192; // scaling down of right shadow (by inner extension)
		U8 bottomShadowIntensity = 255; // scaling down of bottom shadow (by inner extension)

		virtual auto get_component_spacing() -> U32 = 0;
		virtual auto get_section_spacing() -> U32 = 0;

		virtual auto get_minimum_button_width() -> U32 = 0;
		virtual auto get_minimum_button_height() -> U32 = 0;
		virtual void draw_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, bool smallFont, graphics2d::Buffer *icon = nullptr, bool isHover = false, bool isDown = false) = 0;
		virtual void draw_coloured_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon = nullptr, bool isHover = false, bool isDown = false) = 0;
		virtual void draw_toggle_button(graphics2d::Buffer&, graphics2d::Rect, const char *text, bool smallFont, graphics2d::Buffer *icon, bool toggleState, bool isHover = false, bool isDown = false) = 0;
		virtual void draw_coloured_toggle_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon, bool toggleState, bool isHover = false, bool isDown = false) = 0;
		virtual void draw_titlebar_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon = nullptr, bool isHover = false, bool isDown = false);
		virtual void draw_titlebar_toggle_button(graphics2d::Buffer&, graphics2d::Rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::Buffer *icon, bool toggleState, bool isHover = false, bool isDown = false);

		virtual auto get_window_left_margin() -> U32;
		virtual auto get_window_right_margin() -> U32;
		virtual auto get_window_top_margin() -> U32;
		virtual auto get_window_bottom_margin() -> U32;
		virtual auto get_window_min_width() -> U32 = 0;
		virtual auto get_window_min_height() -> U32 = 0;
		struct WindowFrameOptions {
			bool isFocused = true;
			const char *title = "";
			const char *status = "";
			U32 titlebarAreaIndentLeft = 0;
			U32 titlebarAreaIndentRight = 0;
		};
		virtual auto get_shadow_intensity_at(graphics2d::Rect, I32 x, I32 y) -> U8;
		virtual auto get_window_titlebar_area(graphics2d::Rect) -> graphics2d::Rect = 0;
		virtual auto get_window_titlebar_divider_colour(WindowFrameOptions) -> U32 = 0;
		virtual auto get_window_titlebar_widget_margin() -> U32 = 0;
		virtual auto get_window_titlebar_widget_spacing() -> U32 = 0;
		virtual auto get_window_titlebar_widget_large_spacing() -> U32 = 0;
		virtual auto has_window_titlebar_widget_coloured() -> bool = 0;
		virtual auto get_window_titlebar_widget_minimise() -> graphics2d::Buffer&;
		virtual auto get_window_titlebar_widget_maximise() -> graphics2d::Buffer&;
		virtual auto get_window_titlebar_widget_close() -> graphics2d::Buffer&;
		virtual auto get_window_titlebar_widget_restore() -> graphics2d::Buffer&;
		virtual auto get_window_statusbar_area(graphics2d::Rect) -> graphics2d::Rect = 0;
		virtual auto get_window_client_area(graphics2d::Rect) -> graphics2d::Rect = 0;
		virtual auto get_window_solid_area(graphics2d::Rect) -> graphics2d::Rect = 0;
		virtual auto get_window_interact_area(graphics2d::Rect) -> graphics2d::Rect = 0;
		virtual auto get_window_background_colour() -> U32 = 0;
		virtual void draw_window_frame(graphics2d::Buffer&, graphics2d::Rect, WindowFrameOptions options) = 0;
	};
}
