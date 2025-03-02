#include "Buffer.hpp"

#include "Font.hpp"

#include <common/maths/Fixed.hpp>

namespace graphics2d {
	auto Buffer::draw_text(FontSettings fontSettings, const char *text, I32 startX, I32 startY, U32 width, U32 colour, I32 cursorX) -> DrawTextResult {
		auto x = FixedI32::whole(cursorX);
		auto y = FixedI32::whole(startY);

		auto right = FixedI32::whole(startX + (I32)width);

		auto maxX = cursorX;

		auto scale = FixedI32::divide(fontSettings.size, fontSettings.font.size);

		const auto lineheight = (U32)(fontSettings.font.lineHeight * fontSettings.size + 0.5) + fontSettings.lineSpacing;

		Rect updatedArea = {cursorX,startY, cursorX,startY};

		for(const char *c=text;*c;c++){
			switch(*c){
				case '\n':
					x = FixedI32::whole(startX);
					y += lineheight;
				break;
				default: {
					auto character = fontSettings.font.get_character(*c);
					if(!character){
						continue;
					}

					if(character->atlasWidth&&character->atlasHeight){
						auto x1 = x+character->offsetX*(I32)fontSettings.size;
						auto y1 = y-character->offsetY*(I32)fontSettings.size;

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

						draw_msdf(displayX1, displayY1, displayX2-displayX1, displayY2-displayY1, fontSettings.font.atlas, (I32)character->atlasX-skipLeft, (I32)character->atlasY-skipTop, character->atlasWidth+skipLeft+skipRight, character->atlasHeight+skipTop+skipBottom, colour, skipLeft, skipTop, skipRight, skipBottom);

						updatedArea = updatedArea.include({displayX1, displayY1, displayX2, displayY2});
					}

					x += character->advance.cast<I32>()*(I32)fontSettings.font.size*scale + fontSettings.charSpacing;
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
	auto Buffer::measure_text(FontSettings fontSettings, const char *text, I32 startX, I32 startY, U32 width, I32 cursorX) -> DrawTextResult {
		auto x = FixedI32::whole(cursorX);
		auto y = FixedI32::whole(startY);

		auto right = FixedI32::whole(startX + (I32)width);

		auto maxX = cursorX;

		auto scale = FixedI32::divide(fontSettings.size, fontSettings.font.size);

		const auto lineheight = (U32)(fontSettings.font.lineHeight * fontSettings.size + 0.5) + fontSettings.lineSpacing;

		Rect updatedArea = {cursorX,startY, cursorX,startY};

		for(const char *c=text;*c;c++){
			switch(*c){
				case '\n':
					x = FixedI32::whole(startX);
					y += lineheight;
				break;
				default: {
					auto character = fontSettings.font.get_character(*c);
					if(!character){
						continue;
					}

					if(character->atlasWidth&&character->atlasHeight){
						auto x1 = x+character->offsetX*(I32)fontSettings.size;
						auto y1 = y-character->offsetY*(I32)fontSettings.size;

						auto x2 = x1 + scale*(I32)character->atlasWidth;
						auto y2 = y1 + scale*(I32)character->atlasHeight;

						auto displayX1 = x1.round_down();
						auto displayY1 = y1.round_down();
						auto displayX2 = x2.round_up();
						auto displayY2 = y2.round_up();

						updatedArea = updatedArea.include({displayX1, displayY1, displayX2, displayY2});
					}

					x += character->advance.cast<I32>()*(I32)fontSettings.font.size*scale + fontSettings.charSpacing;
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
}
