#include "Buffer.hpp"

#include "Font.hpp"

#include <common/maths/Fixed.hpp>

namespace graphics2d {
	auto Buffer::draw_text(FontSettings fontSettings, const char *text, I32 startX, I32 startY, U32 width, U32 colour, I32 cursorX) -> DrawTextResult {
		auto x = FixedI32::whole(cursorX);
		auto y = FixedI32::whole(startY);

		auto right = FixedI32::whole(startX + (I32)width);
	
		auto x2 = cursorX;
	
		auto scale = FixedI32::divide(fontSettings.size, fontSettings.font.size);
	
		auto lines = 1u;
		auto clipped = false;

		const auto lineHeight = (I32)(fontSettings.font.lineHeight * fontSettings.size + 0.5);
		const auto capHeight = (I32)(fontSettings.font.capHeight * fontSettings.size + 0.5);
		// const auto ascend = (I32)(fontSettings.font.ascender * fontSettings.size + 0.5);
		// const auto descend = (I32)(fontSettings.font.descender * fontSettings.size + 0.5);
	
		Rect updatedArea = {cursorX, startY, cursorX, startY};
	
		for(const char *c=text;*c;c++){
			switch(*c){
				case '\n':
					lines++;
					if(lines>fontSettings.maxLines) break;

					x = FixedI32::whole(startX);
					y += lineHeight+fontSettings.lineSpacing;
				break;
				default: {
					auto character = fontSettings.font.get_character(*c);
					if(!character){
						continue;
					}
	
					const auto xAdvancement = character->advance*(I32)fontSettings.font.size*scale + fontSettings.charSpacing;

					if(c!=text && x+xAdvancement>=right){
						//TODO: proper wordwrapping

						lines++;
						if(lines>fontSettings.maxLines) break;
						
						x = FixedI32::whole(startX);
						y += lineHeight+fontSettings.lineSpacing;
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

					x += xAdvancement;
				}
			}
	
			x2 = max(x2, x.round());
			if(lines>fontSettings.maxLines) {
				clipped = true;
				break;
			}
		}
	
		auto blockWidth = x2-startX;
		auto blockHeight = y.round()+(I32)capHeight;

		return {
			x.round_up(), y.round_up(),
			capHeight,
			lineHeight,
			{startX, startY, startX+blockWidth, startY+blockHeight},
			updatedArea,
			lines,
			clipped
		};
	}
	auto Buffer::measure_text(FontSettings fontSettings, const char *text, U32 width, I32 cursorX) -> DrawTextResult {
		auto x = FixedI32::whole(cursorX);
		auto y = FixedI32::whole(0);

		auto right = FixedI32::whole((I32)min(width, ((1u<<31)-1)/256));
	
		auto blockWidth = cursorX;
	
		auto scale = FixedI32::divide(fontSettings.size, fontSettings.font.size);
	
		auto lines = 1u;
		auto clipped = false;

		const auto lineHeight = (I32)(fontSettings.font.lineHeight * fontSettings.size + 0.5);
		const auto capHeight = (I32)(fontSettings.font.capHeight * fontSettings.size + 0.5);
		// const auto ascend = (I32)(fontSettings.font.ascender * fontSettings.size + 0.5);
		// const auto descend = (I32)(fontSettings.font.descender * fontSettings.size + 0.5);
	
		Rect updatedArea = {cursorX, 0, cursorX, 0};
	
		for(const char *c=text;*c;c++){
			switch(*c){
				case '\n':
					lines++;
					if(lines>fontSettings.maxLines) break;

					x = FixedI32::whole(0);
					y += lineHeight+fontSettings.lineSpacing;
				break;
				default: {
					auto character = fontSettings.font.get_character(*c);
					if(!character){
						continue;
					}

					const auto xAdvancement = character->advance*(I32)fontSettings.font.size*scale + fontSettings.charSpacing;

					if(c!=text && x+xAdvancement>=right){
						//TODO: proper wordwrapping

						lines++;
						if(lines>fontSettings.maxLines) break;
						
						x = FixedI32::whole(0);
						y += lineHeight+fontSettings.lineSpacing;
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

					x += xAdvancement;
				}
			}
	
			blockWidth = max(blockWidth, x.round());
			if(lines>fontSettings.maxLines) {
				clipped = true;
				break;
			}
		}

		auto blockHeight = y.round()+(I32)capHeight;

		return {
			x.round_up(), y.round_up(),
			capHeight,
			lineHeight,
			{0, 0, blockWidth, blockHeight},
			updatedArea,
			lines,
			clipped
		};
	}
}
