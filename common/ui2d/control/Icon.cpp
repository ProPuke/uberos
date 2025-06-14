#include "Icon.hpp"

#include <common/graphics2d/font.hpp>

namespace ui2d {
	namespace control {
		namespace {
			const auto labelGap = 0;
			const auto padding = 4;
			const auto minIconSize = 32;
			// const auto minIconSize = 16;

			auto get_font_size_for_iconSize(U32 iconSize) -> U32 {
				return iconSize<24?10u:iconSize<32?11u:iconSize<64?12u:iconSize<128?13u:14u;
			}
		}

		void Icon::set_icon_size(I32 set) {
			if(iconSize==set) return;

			iconSize = set;
		}

		void Icon::set_selected(bool set) {
			if(isSelected==set) return;

			isSelected = set;
		}

		void Icon::redraw(bool flush) {
			if(!isVisible) return;

			const auto rescaleIcon = false;

			I32 iconSize = this->iconSize;
			I32 labelLineHeight;
			I32 labelHeight;
			auto labelFontSettings = graphics2d::Buffer::FontSettings{
				.font=*graphics2d::font::default_sans,
				.size=14,
				.lineSpacing=-2,
				.clipped=true,
				.maxLines=2,
			};

			if(iconSize==0){
				U32 fontSize = get_font_size_for_iconSize(9999);

				while(true) {
					labelLineHeight = (I32)(graphics2d::font::default_sans->lineHeight * fontSize + 0.5)-2;
					labelHeight = labelLineHeight*2;
					iconSize = maths::min(rect.width()-padding*2, rect.height()-labelHeight-labelGap-padding*2);

					auto newFontSize = get_font_size_for_iconSize(iconSize);
					if(newFontSize>=fontSize) break;

					fontSize = newFontSize;
				};

				labelFontSettings.size = fontSize;

			}else{
				U32 fontSize = get_font_size_for_iconSize(iconSize);

				labelLineHeight = (I32)(graphics2d::font::default_sans->lineHeight * fontSize + 0.5)-2;
				labelHeight = labelLineHeight*2;
				labelFontSettings.size = fontSize;
			}

			auto iconBuffer = rescaleIcon?icon.get_size_or_larger_or_smaller(iconSize):icon.get_size_or_smaller(iconSize);
			if(iconBuffer&&!rescaleIcon){
				iconSize = iconBuffer->height;
			}

			auto displayRect = graphics2d::Rect{0, 0, iconSize+padding*2, iconSize+labelGap+labelHeight+padding*2};
			displayRect = displayRect.offset(rect.x1+(rect.width()-displayRect.width())/2, rect.y1+(rect.height()-displayRect.height())/2);

			const auto iconRect = graphics2d::Rect{displayRect.x1+padding, displayRect.y1+padding, displayRect.x1+padding+iconSize, displayRect.y1+padding+iconSize};
			const auto labelRect = graphics2d::Rect{
				iconRect.x1+padding, maths::min(iconRect.y2+labelGap, rect.y2-padding-labelHeight),
				iconRect.x2-padding, maths::min(iconRect.y2+labelGap+labelHeight, rect.y2-padding)
			};

			U32 corners[3];
			graphics2d::create_diagonal_corner(2, corners);

			const auto backgroundColour = gui.theme->get_window_background_colour();

			gui.buffer.draw_rect(displayRect, backgroundColour);
			if(isSelected||isHover||isPressed){
				gui.buffer.draw_rect(displayRect, graphics2d::blend_colours(backgroundColour, 0xff000000-(0x16*(isSelected+isHover+isPressed))<<24), corners, corners, corners, corners);
			}

			if(iconBuffer){
				gui.buffer.draw_scaled_buffer_blended(iconRect.x1, iconRect.y1, iconRect.width(), iconRect.height(), *iconBuffer, 0, 0, iconBuffer->width, iconBuffer->height, {});
			}

			if(labelRect.height()>0){
				auto size = gui.buffer.measure_text(labelFontSettings, label, labelRect.width());
				gui.buffer.draw_text(labelFontSettings, label, labelRect.x1, labelRect.y1+labelLineHeight, labelRect.width(), 0x000000, labelRect.x1+(labelRect.width()-size.rect.width())/2);
			}

			if(flush){
				gui.update_area(displayRect);
			}
		}

		auto Icon::get_min_size() -> IVec2 {
			const auto fontSize = get_font_size_for_iconSize(iconSize);
			const auto labelLineHeight = (I32)(graphics2d::font::default_sans->lineHeight * fontSize + 0.5)-2;
			const auto labelHeight = labelLineHeight*2;

			return {padding+iconSize+padding, padding+iconSize+labelGap+labelHeight+padding};
		}

		auto Icon::get_max_size() -> IVec2 {
			return get_min_size();
		}

		void Icon::on_mouse_pressed(I32 x, I32 y, U32 button) {
			Super::on_mouse_pressed(x, y, button);

			if(button==0&&rect.contains(x, y)){
				on_pressed();
			}
		}

		void Icon::on_mouse_released(I32 x, I32 y, U32 button) {
			auto wasClicked = false;
			auto wasReleased = false;

			if(button==0&&isPressed){
				wasReleased = true;
			}

			if(rect.contains(x, y)){
				if(isPressed){
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
