#include "Buffer.hpp"

#include "Font.hpp"

#include <common/maths/Fixed.hpp>

namespace graphics2d {
	void Buffer::draw_text(Font &font, const char *text, I32 startX, I32 startY, U32 size, U32 colour) {
		auto x = FixedI32::whole(startX);
		auto y = FixedI32::whole(startY);

		auto scale = FixedI32::divide(size, font.size);

		for(const char *c=text;*c;c++){
			auto character = font.get_character(*c);
			if(!character){
				continue;
			}

			if(character->atlasWidth&&character->atlasHeight){
				auto x1 = x+character->offsetX.cast<I32>()*(I32)size;
				auto y1 = y-character->offsetY.cast<I32>()*(I32)size;

				auto x2 = x1 + scale*(I32)character->atlasWidth;
				auto y2 = y1 + scale*(I32)character->atlasHeight;

				auto displayX1 = x1.round_down();
				auto displayY1 = y1.round_down();
				auto displayX2 = x2.round_up();
				auto displayY2 = y2.round_up();

				auto skipLeft = 0;
				auto skipTop = 0;
				auto skipRight = 0;
				auto skipBottom = 0;

				if(scale<=FixedI32::divide(1, 2)){
					skipLeft   = ((x1-displayX1) / scale).round();
					skipTop    = ((y1-displayY1) / scale).round();
					skipRight  = ((displayX2-x2) / scale).round();
					skipBottom = ((displayY2-y2) / scale).round();
				}

				draw_msdf(displayX1, displayY1, displayX2-displayX1, displayY2-displayY1, font.atlas, (I32)character->atlasX-skipLeft, (I32)character->atlasY-skipTop, character->atlasWidth+skipLeft+skipRight, character->atlasHeight+skipTop+skipBottom, colour, skipLeft, skipTop, skipRight, skipBottom);
			}

			x += character->advance.cast<I32>()*(I32)font.size*scale;
		}
	}
}