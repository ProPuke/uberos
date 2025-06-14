#include "Win9x.hpp"

#include <kernel/memory.hpp>

#include <common/graphics2d/font.hpp>

namespace ui2d::image {
	namespace widgets {
		extern graphics2d::MultisizeIcon close;
		extern graphics2d::MultisizeIcon maximise;
		extern graphics2d::MultisizeIcon restore;
		extern graphics2d::MultisizeIcon minimise;
	}
}

namespace ui2d {
	namespace theme {
		namespace {
			// U32 buttonAlternateFace = 0xc0c0c0;
			U32 buttonDkShadow = 0x000000;
			U32 buttonFace = 0xc0c0c0;
			U32 buttonHilight = 0xffffff;
			U32 buttonLight = 0xdfdfdf;
			U32 buttonShadow = 0x808080;
			U32 buttonText = 0x000000;
			// U32 activeBorder = 0xd4d0c8;
			// U32 appWorkspace = 0x808080;
			// U32 background = 0x3a6ea5;
			// U32 inactiveBorder = 0xd4d0c8;
			// U32 scrollbar = 0xd4d0c8;
			// U32 window = 0xffffff;
			// U32 windowFrame = 0x000000;
			U32 windowText = 0x000000;
			U32 activeTitle = 0x0a246a;
			U32 gradientActiveTitle = 0xa6caf0;
			U32 gradientInactiveTitle = 0xc0c0c0;
			U32 inactiveTitle = 0x808080;
			U32 inactiveTitleText = 0xd4d0c8;
			U32 titleText = 0xffffff;
			// U32 menu = 0xd4d0c8;
			// U32 menuBar = 0xc0c0c0;
			// U32 menuHilight = 0x000080;
			// U32 menuText = 0x000000;
			// U32 grayText = 0x808080;
			// U32 hilight = 0x0a246a;
			// U32 hilightText = 0xffffff;
			// U32 hotTrackingColor = 0x000080;
			// U32 infoText = 0x000000;
			// U32 infoWindow = 0xffffe1;

			auto titlebarHeight = 18;
			// auto titlebarHeight = 20;
			auto statusbarHeight = 18;
			// auto statusbarHeight = 20;

			void draw_window_bevel(graphics2d::Buffer &buffer, graphics2d::Rect rect, bool isDown, bool filled = true, U32 colour = theme::buttonFace);
			void draw_button_bevel(graphics2d::Buffer &buffer, graphics2d::Rect rect, bool isDown, bool filled = true, U32 colour = theme::buttonFace);
			void draw_thin_bevel(graphics2d::Buffer &buffer, graphics2d::Rect rect, bool isDown, bool filled = true, U32 colour = theme::buttonFace);

			void draw_window_bevel(graphics2d::Buffer &buffer, graphics2d::Rect rect, bool isDown, bool filled, U32 colour) {
				if(rect.width()<5||rect.height()<5) return draw_thin_bevel(buffer, rect, isDown, filled, colour);

				const auto face = graphics2d::blend_colours(theme::buttonFace, colour&0xffffff)|colour&0xff000000;
				const auto light = graphics2d::apply_colour_difference(isDown?theme::buttonDkShadow:theme::buttonLight, theme::buttonFace, face);
				const auto hilight = graphics2d::apply_colour_difference(isDown?theme::buttonShadow:theme::buttonHilight, theme::buttonFace, face);
				const auto shadow = graphics2d::apply_colour_difference(isDown?theme::buttonHilight:theme::buttonShadow, theme::buttonFace, face);
				const auto dhShadow = graphics2d::apply_colour_difference(isDown?theme::buttonLight:theme::buttonDkShadow, theme::buttonFace, face);

				buffer.set(rect.x1, rect.y1, light, rect.width()-1);
				buffer.set(rect.x2-1, rect.y1, dhShadow, 1);

				buffer.set(rect.x1, rect.y1+1, light, 1);
				buffer.set(rect.x1+1, rect.y1+1, hilight, rect.width()-2-1);
				buffer.set(rect.x2-2, rect.y1+1, shadow, 1);
				buffer.set(rect.x2-1, rect.y1+1, dhShadow, 1);

				for(auto y=rect.y1+2; y<rect.y2-2; y++){
					buffer.set(rect.x1, y, light, 1);
					buffer.set(rect.x1+1, y, hilight, 1);
					if(filled){
						buffer.set(rect.x1+2, y, face, rect.width()-2-2);
					}
					buffer.set(rect.x2-2, y, shadow, 1);
					buffer.set(rect.x2-1, y, dhShadow, 1);
				}

				buffer.set(rect.x1, rect.y2-2, light, 1);
				buffer.set(rect.x1+1, rect.y2-2, shadow, rect.width()-1-1);
				buffer.set(rect.x2-1, rect.y2-2, dhShadow, 1);

				buffer.set(rect.x1, rect.y2-1, dhShadow, rect.width());
			}

			void draw_button_bevel(graphics2d::Buffer &buffer, graphics2d::Rect rect, bool isDown, bool filled, U32 colour) {
				if(rect.width()<5||rect.height()<5) return draw_thin_bevel(buffer, rect, isDown, filled, colour);

				const auto face = graphics2d::blend_colours(theme::buttonFace, colour&0xffffff)|colour&0xff000000;
				const auto hilight = graphics2d::apply_colour_difference(isDown?theme::buttonDkShadow:theme::buttonHilight, theme::buttonFace, face);
				const auto light = graphics2d::apply_colour_difference(isDown?theme::buttonShadow:theme::buttonLight, theme::buttonFace, face);
				const auto shadow = graphics2d::apply_colour_difference(isDown?theme::buttonLight:theme::buttonShadow, theme::buttonFace, face);
				const auto dkShadow = graphics2d::apply_colour_difference(isDown?theme::buttonHilight:theme::buttonDkShadow, theme::buttonFace, face);

				buffer.set(rect.x1, rect.y1, hilight, rect.width()-1);
				buffer.set(rect.x2-1, rect.y1, shadow, 1);

				buffer.set(rect.x1, rect.y1+1, hilight, 1);
				buffer.set(rect.x1+1, rect.y1+1, light, rect.width()-2-1);
				buffer.set(rect.x2-2, rect.y1+1, shadow, 1);
				buffer.set(rect.x2-1, rect.y1+1, dkShadow, 1);

				for(auto y=rect.y1+2; y<rect.y2-2; y++){
					buffer.set(rect.x1, y, hilight, 1);
					buffer.set(rect.x1+1, y, light, 1);
					if(filled){
						buffer.set(rect.x1+2, y, face, rect.width()-2-2);
					}
					buffer.set(rect.x2-2, y, shadow, 1);
					buffer.set(rect.x2-1, y, dkShadow, 1);
				}

				buffer.set(rect.x1, rect.y2-2, hilight, 1);
				buffer.set(rect.x1+1, rect.y2-2, shadow, rect.width()-1-1);
				buffer.set(rect.x2-1, rect.y2-2, dkShadow, 1);

				buffer.set(rect.x1, rect.y2-1, dkShadow, rect.width());
			}

			void draw_thin_bevel(graphics2d::Buffer &buffer, graphics2d::Rect rect, bool isDown, bool filled, U32 colour) {
				if(rect.width()<4||rect.height()<4) return;

				const auto face = graphics2d::blend_colours(theme::buttonFace, colour&0xffffff)|colour&0xff000000;
				const auto hilight = graphics2d::apply_colour_difference(isDown?theme::buttonShadow:theme::buttonHilight, theme::buttonFace, face);
				const auto light = graphics2d::apply_colour_difference(theme::buttonLight, theme::buttonFace, face);
				const auto shadow = graphics2d::apply_colour_difference(isDown?theme::buttonHilight:theme::buttonShadow, theme::buttonFace, face);

				buffer.set(rect.x1, rect.y1, hilight, rect.width()-1);
				buffer.set(rect.x2-1, rect.y1, light, 1);

				buffer.set(rect.x1, rect.y1+1, hilight, 1);
				buffer.set(rect.x1+1, rect.y1+1, face, rect.width()-1-1);
				buffer.set(rect.x2-1, rect.y1+1, shadow, 1);

				for(auto y=rect.y1+1; y<rect.y2-1; y++){
					buffer.set(rect.x1, y, hilight, 1);
					if(filled){
						buffer.set(rect.x1+1, y, face, rect.width()-1-1);
					}
					buffer.set(rect.x2-1, y, shadow, 1);
				}

				buffer.set(rect.x1, rect.y2-1, light, 1);
				buffer.set(rect.x1+1, rect.y2-1, shadow, rect.width()-1);
			}
		}

		/**/ Win9x::Win9x() {}

		auto Win9x::get_component_spacing() -> I32 { return 7; }
		auto Win9x::get_section_spacing() -> I32 { return 11; }

		auto Win9x::get_minimum_button_width() -> I32 {
			return 100;
		}
		auto Win9x::get_minimum_button_height() -> I32 {
			return 28;
		}

		void Win9x::draw_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool isHover, bool isDown) {
			return draw_coloured_button(buffer, rect, 0xffffff, theme::buttonFace, 255, text, smallFont, icon, isHover, isDown);
		}

		void Win9x::draw_coloured_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool isHover, bool isDown) {
			if(rect.width()<5||rect.height()<5) return;

			auto bgColour = graphics2d::blend_colours(backgroundColour, colour, opacity);
			draw_button_bevel(buffer, rect, isDown, true, bgColour);

			auto contentRect = rect.cropped(3,3,3,3);
			auto textRect = contentRect;

			if(auto iconBuffer = icon.get_size_or_larger_or_smaller(contentRect.height()-2)){
				if(iconBuffer->height>=4){
					if((I32)iconBuffer->height<=contentRect.height()+2){
						buffer.draw_buffer_blended(contentRect.x1-1+((contentRect.width()+2)-(I32)iconBuffer->width)/2, contentRect.y1-1+(contentRect.height()+2-(I32)iconBuffer->height)/2+(isDown?1:0), 0, 0, iconBuffer->width, iconBuffer->height, *iconBuffer, opacity);
					}else{
						const auto trim = 0; // trim pixels off the edges to ensure a tighter fit
						const auto iconHeight = contentRect.height()+2;
						const auto iconWidth = ((I32)iconBuffer->width-trim*2) * iconHeight/((I32)iconBuffer->height-trim*2);
						buffer.draw_scaled_buffer_blended(contentRect.x1-1+((contentRect.width()+2)-iconWidth)/2, contentRect.y1-1, iconWidth, iconHeight, *iconBuffer, trim, trim, iconBuffer->width-trim*2, iconBuffer->height-trim*2, {}, opacity);
					}
				}
			}

			const auto textColour = graphics2d::multiply_colours(colour, buttonText);

			const graphics2d::Buffer::FontSettings fontSettings{
				.font=*graphics2d::font::default_sans,
				.size=smallFont?12u:14u,
				.lineSpacing=-2,
				.clipped=true,
				.maxLines=1
			};
			const auto measurements = buffer.measure_text(fontSettings, text, textRect.width());
			buffer.draw_text(fontSettings, text, textRect.x1+(textRect.width()-measurements.rect.width())/2, textRect.y1+(textRect.height()+measurements.capHeight)/2+(isDown?1:0), textRect.width(), textColour);
		}

		void Win9x::draw_coloured_toggle_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool toggleState, bool isHover, bool isDown) {
			return draw_coloured_button(buffer, rect, backgroundColour, colour, opacity, text, smallFont, icon, isHover, isDown||toggleState);
		}

		void Win9x::draw_toggle_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool toggleState, bool isHover, bool isDown) {
			return draw_coloured_toggle_button(buffer, rect, 0xffffff, theme::buttonFace, 255, text, smallFont, icon, toggleState, isHover, isDown);
		}

		auto Win9x::get_window_min_width() -> U32 {
			return 100;
		}
		auto Win9x::get_window_min_height() -> U32 {
			return 4+titlebarHeight+4+statusbarHeight+2;
		}

		auto Win9x::get_window_titlebar_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return {rect.x1+(I32)leftShadow+4, rect.y1+(I32)topShadow+4, rect.x2-(I32)rightShadow-4, rect.y1+(I32)topShadow+4+titlebarHeight};
		}

		auto Win9x::get_window_statusbar_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return {rect.x1+(I32)leftShadow+4, rect.y2-(I32)bottomShadow-4-statusbarHeight, rect.x2-(I32)rightShadow-4, rect.y2-(I32)bottomShadow-4};
		}

		auto Win9x::get_window_titlebar_divider_colour(WindowFrameOptions options) -> U32 {
			return 0xff000000;
		}

		auto Win9x::get_window_titlebar_widget_margin() -> U32 {
			return 2;
		}

		auto Win9x::get_window_titlebar_widget_spacing() -> U32 {
			return 0;
		}

		auto Win9x::get_window_titlebar_widget_large_spacing() -> U32 {
			return 2;
		}

		auto Win9x::has_window_titlebar_widget_coloured() -> bool {
			return false;
		}

		auto Win9x::get_window_titlebar_widget_minimise() -> graphics2d::MultisizeIcon& {
			return image::widgets::minimise;
		}
		auto Win9x::get_window_titlebar_widget_maximise() -> graphics2d::MultisizeIcon& {
			return image::widgets::maximise;
		}
		auto Win9x::get_window_titlebar_widget_close() -> graphics2d::MultisizeIcon& {
			return image::widgets::close;
		}
		auto Win9x::get_window_titlebar_widget_restore() -> graphics2d::MultisizeIcon& {
			return image::widgets::restore;
		}

		auto Win9x::get_window_client_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return rect.cropped(leftShadow+4, topShadow+4+titlebarHeight+2, rightShadow+4, bottomShadow+4+statusbarHeight+2);
		}

		auto Win9x::get_window_solid_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return rect.cropped(leftShadow, topShadow, rightShadow, bottomShadow);
		}

		auto Win9x::get_window_interact_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return rect.cropped(leftShadow, topShadow, rightShadow, bottomShadow);
		}

		auto Win9x::get_window_background_colour() -> U32 {
			return buttonFace;
		}

		void Win9x::draw_window_frame(graphics2d::Buffer &buffer, graphics2d::Rect rect, WindowFrameOptions options) {
			const auto titlebarArea = get_window_titlebar_area(rect);
			const auto statusbarArea = get_window_statusbar_area(rect);
			const auto solidArea = get_window_solid_area(rect);
			const auto clientArea = get_window_client_area(rect);

			buffer.draw_rect({solidArea.x1+2, solidArea.y1+2, solidArea.x2-2, clientArea.y1}, buttonFace);
			buffer.draw_rect({solidArea.x1+2, clientArea.y1, clientArea.x1, clientArea.y2}, buttonFace);
			buffer.draw_rect({clientArea.x2, clientArea.y1, solidArea.x2-2, clientArea.y2}, buttonFace);
			buffer.draw_rect({solidArea.x1+2, clientArea.y2, solidArea.x2-2, solidArea.y2-2}, buttonFace);

			draw_window_bevel(buffer, solidArea, false, false);

			if(true){ // gradient
				for(auto y=titlebarArea.y1; y<titlebarArea.y2; y++){
					for(auto x=titlebarArea.x1; x<titlebarArea.x2; x++){
						U8 phase = 255*(x-titlebarArea.x1)/titlebarArea.width();
						buffer.set(x, y, graphics2d::blend_colours(options.isFocused?activeTitle:inactiveTitle, options.isFocused?gradientActiveTitle:gradientInactiveTitle, phase));
					}
				}
			}else{ // solid
				buffer.draw_rect(titlebarArea, options.isFocused?activeTitle:inactiveTitle);
			}

			for(auto y=rect.y1;y<rect.y1+(I32)topShadow&&y<rect.y1+rect.height()/2;y++)
			for(auto x=rect.x1;x<rect.x2;x++){
				buffer.set(x, y, 0x000000|(255-get_shadow_intensity_at(rect, x, y)<<24));
			}

			for(auto y=rect.y1+(I32)topShadow;y<rect.y2-(I32)bottomShadow; y++){
				for(auto x=rect.x1; x<rect.x1+(I32)leftShadow; x++){
					buffer.set(x, y, 0x000000|(255-get_shadow_intensity_at(rect, x, y)<<24));
				}
				for(auto x=rect.x2-(I32)rightShadow;x<rect.x2; x++){
					buffer.set(x, y, 0x000000|(255-get_shadow_intensity_at(rect, x, y)<<24));
				}
			}

			for(auto y=max(rect.y1+rect.height()/2, rect.y2-(I32)bottomShadow);y<rect.y2;y++)
			for(auto x=rect.x1;x<rect.x2;x++){
				buffer.set(x, y, 0x000000|(255-get_shadow_intensity_at(rect, x, y)<<24));
			}

			buffer.draw_text({
				.font=*graphics2d::font::default_sans,
				.size=14u,
				.clipped=true,
				.maxLines=1
			}, options.title, titlebarArea.x1+options.titlebarAreaIndentLeft, titlebarArea.y2-5, titlebarArea.width()-options.titlebarAreaIndentRight, options.isFocused?titleText:inactiveTitleText);

			draw_thin_bevel(buffer, statusbarArea, true);
			buffer.draw_text({
				.font=*graphics2d::font::default_sans,
				.size=14u,
				.clipped=true,
				.maxLines=1
			}, options.status, statusbarArea.x1+1, statusbarArea.y2-5, statusbarArea.width()-2, windowText);

			// draw_window_bevel(buffer, clientArea.cropped(-2,-2,-2,-2), true);
		}

		auto Win9x::get_box_client_area(graphics2d::Rect rect, BoxType boxType) -> graphics2d::Rect {
			auto spacing = get_component_spacing();
			return rect.cropped(2+spacing, 2+spacing, 2+spacing, 2+spacing);
		}

		void Win9x::draw_box(graphics2d::Buffer &buffer, graphics2d::Rect rect, BoxType boxType) {
			switch(boxType){
				case BoxType::default_:
					buffer.draw_rect_outline(rect, buttonShadow, 2);
				break;
				case BoxType::inset:
					draw_thin_bevel(buffer, rect, true, false);
				break;
			}
		}
	}
}