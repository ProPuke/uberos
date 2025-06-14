#include "Theme.hpp"

#include <common/graphics2d/MultisizeIcon.hpp>

namespace ui2d::image {
	namespace widgets {
		extern graphics2d::MultisizeIcon close;
		extern graphics2d::MultisizeIcon maximise;
		extern graphics2d::MultisizeIcon restore;
		extern graphics2d::MultisizeIcon minimise;
	}
}

namespace ui2d {
	namespace {
		const auto enableTransparency = true;
	}

	/**/ Theme::Theme():
		leftShadow(enableTransparency?8:0),
		rightShadow(enableTransparency?8:0),
		topShadow(enableTransparency?8:0),
		bottomShadow(enableTransparency?8:0)
	{}

	auto Theme::get_window_titlebar_widget_minimise() -> graphics2d::MultisizeIcon& {
		return image::widgets::minimise;
	}
	auto Theme::get_window_titlebar_widget_maximise() -> graphics2d::MultisizeIcon& {
		return image::widgets::maximise;
	}
	auto Theme::get_window_titlebar_widget_close() -> graphics2d::MultisizeIcon& {
		return image::widgets::close;
	}
	auto Theme::get_window_titlebar_widget_restore() -> graphics2d::MultisizeIcon& {
		return image::widgets::restore;
	}

	auto Theme::get_shadow_intensity_at(graphics2d::Rect rect, I32 x, I32 y) -> U8 {
		const auto width = rect.width();
		const auto height = rect.height();
		const auto clientWidth = width-leftShadow-rightShadow;
		const auto clientHeight = height-topShadow-bottomShadow;

		x -= rect.x1;
		y -= rect.y1;

		U8 intensity = 255;

		//we lengthen it inward when the intensity is turned down (so it fades in nicely at the bottom)
		auto topShadowLength = (I32)topShadow * 255/(I32)topShadowIntensity;
		auto leftShadowLength = (I32)leftShadow * 255/(I32)leftShadowIntensity;
		auto rightShadowLength = (I32)rightShadow * 255/(I32)rightShadowIntensity;
		auto bottomShadowLength = (I32)bottomShadow * 255/(I32)bottomShadowIntensity;

		if(x<(I32)leftShadowLength&&x<(I32)width/2){
			intensity = intensity * (x+1)/leftShadowLength;

		}else if(x>=(I32)width-rightShadowLength&&x>=(I32)width/2){
			intensity = intensity * (leftShadow+clientWidth+rightShadow-x)/rightShadowLength;
		}

		{
			if(y<(I32)topShadow){
				intensity = intensity * (y+1)/topShadowLength;

			}else if(y>=(I32)topShadow+(I32)clientHeight){
				intensity = intensity * (topShadow+clientHeight+bottomShadow-y)/bottomShadowLength;
			}
		}

		// const auto topShadow = bottomShadow*2; // top fade twice of bottom

		// if(y<(I32)topShadow){
		// 	// use for top fade, as well
		// 	intensity = intensity * (y+1)/topShadow;

		// }else if(y>=clientHeight){
		// 	intensity = intensity * (clientHeight+bottomShadow-y)/bottomShadow;
		// }

		return intensity*shadowIntensity/255;
	}

	auto Theme::get_window_left_margin() -> U32 {
		return leftShadow;
	}
	auto Theme::get_window_right_margin() -> U32 {
		return rightShadow;
	}
	auto Theme::get_window_top_margin() -> U32 {
		return topShadow;
	}
	auto Theme::get_window_bottom_margin() -> U32 {
		return bottomShadow;
	}

	void Theme::draw_titlebar_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool isHover, bool isDown) {
		draw_coloured_button(buffer, rect, backgroundColour, colour, opacity, text, smallFont, icon, isHover, isDown);
	}

	void Theme::draw_titlebar_toggle_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool toggleState, bool isHover, bool isDown) {
		draw_coloured_toggle_button(buffer, rect, backgroundColour, colour, opacity, text, smallFont, icon, toggleState, isHover, isDown);
	}
}
