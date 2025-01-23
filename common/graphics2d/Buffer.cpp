#include "Buffer.hpp"

#include "Font.hpp"

#include <common/maths/Fixed.hpp>

namespace graphics2d {
	auto Buffer::draw_text(Font &font, const char *text, I32 startX, I32 startY, U32 width, U32 size, U32 colour, U32 lineheight, I32 cursorX) -> DrawTextResult {
		auto x = FixedI32::whole(cursorX);
		auto y = FixedI32::whole(startY);

		auto right = FixedI32::whole(startX + (I32)width);

		auto maxX = cursorX;

		auto scale = FixedI32::divide(size, font.size);

		Rect updatedArea = {cursorX,startY, cursorX,startY};

		for(const char *c=text;*c;c++){
			switch(*c){
				case '\n':
					x = FixedI32::whole(startX);
					y += lineheight;
				break;
				default: {
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

						updatedArea.x1 = min(updatedArea.x1, displayX1);
						updatedArea.y1 = min(updatedArea.y1, displayY1);
						updatedArea.x2 = max(updatedArea.x2, displayX2);
						updatedArea.y2 = max(updatedArea.y2, displayY2);
					}

					x += character->advance.cast<I32>()*(I32)font.size*scale;
					if(x>=right){
						//TODO: proper wordwrapping
						x = FixedI32::whole(startX);
						y += lineheight;
					}
				}
			}

			maxX = max(maxX, x.round_up());
		}

		return {
			x.round_up(), y.round_up(),
			maxX,
			updatedArea,
		};
	}
	auto Buffer::measure_text(Font &font, const char *text, I32 startX, I32 startY, U32 size, U32 lineheight, I32 cursorX) -> DrawTextResult {
		auto x = FixedI32::whole(cursorX);
		auto y = FixedI32::whole(startY);

		auto maxX = cursorX;

		auto scale = FixedI32::divide(size, font.size);

		Rect updatedArea = {cursorX,startY, cursorX,startY};

		for(const char *c=text;*c;c++){
			switch(*c){
				case '\n':
					x = FixedI32::whole(startX);
					y += lineheight;
				break;
				default: {
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

						updatedArea.x1 = min(updatedArea.x1, displayX1);
						updatedArea.y1 = min(updatedArea.y1, displayY1);
						updatedArea.x2 = max(updatedArea.x2, displayX2);
						updatedArea.y2 = max(updatedArea.y2, displayY2);
					}

					x += character->advance.cast<I32>()*(I32)font.size*scale;
				}
			}

			maxX = max(maxX, x.round_up());
		}

		return {
			x.round_up(), y.round_up(),
			maxX,
			updatedArea,
		};
	}
}
