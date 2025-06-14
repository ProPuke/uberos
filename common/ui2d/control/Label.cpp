#include "Label.hpp"

#include <common/graphics2d/font.hpp>

namespace ui2d {
	namespace control {
		void Label::redraw(bool flush) {
			if(!isVisible) return;

			gui.buffer.draw_rect(rect, gui.theme->get_window_background_colour());

			auto fontSettings = graphics2d::Buffer::FontSettings{
				.font = *graphics2d::font::default_sans,
				.size = fontSize
			};

			const auto ascender = (U32)(fontSettings.font.ascender * fontSize + 0.5);

			auto colour = this->colour.get_or(0x333333); //TODO: get default theme text colour

			gui.buffer.draw_text(fontSettings, text, rect.x1, rect.y1+ascender, rect.width(), colour);

			if(flush){
				gui.update_area(rect);
			}
		}

		void Label::set_text(const char *set) {
			if(text==set) return;

			text = set;
		}

		void Label::set_fontSize(U32 set) {
			if(fontSize==set) return;

			fontSize = set;
		}

		void Label::set_colour(U32 set) {
			if(colour&&*colour==set) return;

			colour = set;
		}

		auto Label::get_min_size() -> IVec2 {
			auto fontSettings = graphics2d::Buffer::FontSettings{
				.font = *graphics2d::font::default_sans,
				.size = fontSize
			};

			const auto size = gui.buffer.measure_text(fontSettings, text);
			return {size.rect.width(), size.rect.height()-size.capHeight+size.lineHeight};

			// auto lineHeight = (U32)(fontSettings.font.lineHeight*fontSize+0.5);
			// return {4, lineHeight};
		}

		auto Label::get_max_size() -> IVec2 {
			return {0x7fff'ffff, get_min_size().y};
		}
	}
}
