#include "Clean.hpp"

#include <common/graphics2d/font.hpp>

namespace ui2d {
	namespace theme {
		namespace {
			const U32 windowBackgroundColour = 0xeeeeee;
			const U32 windowBorderColour = 0xbcbcbc;
		}

		auto Clean::get_component_spacing() -> U32 { return 7; }
		auto Clean::get_section_spacing() -> U32 { return 11; }

		auto Clean::get_minimum_button_width() -> U32 {
			return 100;
		}
		auto Clean::get_minimum_button_height() -> U32 {
			return 28;
		}

		void Clean::draw_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, const char *text, graphics2d::Buffer *icon, bool isHover, bool isDown) {
			return draw_coloured_button(buffer, rect, windowBackgroundColour, 0xdddddd, 0xff, text, icon, isHover, isDown);
		}

		void Clean::draw_coloured_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, graphics2d::Buffer *icon, bool isHover, bool isDown) {
			if(rect.width()<6||rect.height()<6) return;

			// const auto trans = colour>>24;

			auto bgColour = isDown?colour:graphics2d::blend_colours(colour, graphics2d::premultiply_colour(0xbbffffff));
			// auto borderColour = graphics2d::blend_colours(bgColour, isHover?isDown?0xaa000000:0x82000000:0xbb000000);
			auto borderColour = isHover?0x99000000:0xdd000000;
			auto textColour = isHover?0x222222:0x444444;
			auto padding = 2;

			// blend both the possibly transparent colours against the bg AND mix in total opacity
			bgColour = graphics2d::blend_colours(backgroundColour, bgColour, opacity);
			borderColour = graphics2d::blend_colours(backgroundColour, borderColour, opacity);
			textColour = graphics2d::blend_colours(bgColour, textColour, opacity);

			U32 corners[3];
			graphics2d::create_diagonal_corner(2, corners);

			auto innerRect = rect.cropped(1,1,1,1);

			buffer.draw_rect(rect, bgColour, corners, corners, corners, corners);
			buffer.draw_rect_outline(rect, borderColour, 1, corners, corners, corners, corners);

			auto fontSettings = graphics2d::Buffer::FontSettings{
				.font=*graphics2d::font::default_sans,
				.size=14,
				.lineSpacing=-4
			};

			auto textSize = buffer.measure_text(fontSettings, text);
			auto textWidth = min(textSize.blockWidth, innerRect.width()-padding-padding);
			auto textHeight = min(textSize.blockHeight, innerRect.height()-padding-padding);

			if(icon){
				const auto iconSize = icon->height;
				const auto iconOpacity = opacity*min(255u, (max(6u, iconSize)-6)*255/4)/255 * (isHover?255:77)/255;
				buffer.draw_buffer_blended(innerRect.x1+innerRect.width()/2-icon->width/2, innerRect.y1+innerRect.height()/2-icon->height/2+(isDown?1:0), 0, 0, icon->width, icon->height, *icon, iconOpacity);
			}

			buffer.draw_text(fontSettings, text, innerRect.x1+innerRect.width()/2-textWidth/2, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0), textWidth, textColour);
		}

		void Clean::draw_coloured_toggle_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, graphics2d::Buffer *icon, bool toggleState, bool isHover, bool isDown) {
			if(true){
				if(rect.width()<6||rect.height()<6) return;

				// const auto trans = colour>>24;

				auto bgColour = isDown||toggleState?colour:graphics2d::blend_colours(colour, graphics2d::premultiply_colour(0xbbffffff));
				// auto borderColour = graphics2d::blend_colours(bgColour, toggleState&&!isDown?0x66000000:isHover?isDown?0xaa000000:0x82000000:0xbb000000);
				auto borderColour = toggleState&&!isDown?0x66000000:isHover?0x99000000:0xdd000000;
				auto textColour = isHover?0x222222:toggleState?0x444444:graphics2d::premultiply_colour(0x66444444);
				auto stateColour = toggleState||isDown?0x11ffff:backgroundColour;
				const auto padding = 2;

				bgColour = graphics2d::blend_colours(backgroundColour, bgColour, opacity);
				borderColour = graphics2d::blend_colours(backgroundColour, borderColour, opacity);
				textColour = graphics2d::blend_colours(bgColour, textColour, opacity);
				stateColour = graphics2d::blend_colours(backgroundColour, stateColour, opacity);

				const auto cornerSize = 6;

				U32 corners[3];
				graphics2d::create_diagonal_corner(2, corners);

				U32 bigCorner[cornerSize+4];
				graphics2d::create_diagonal_corner(cornerSize+3, bigCorner);

				auto innerRect = rect.cropped(1,1,1,1);

				buffer.draw_rect(rect, bgColour, corners, corners, corners, bigCorner);
				// buffer.draw_rect_outline(rect, toggleState?stateColour:borderColour, 1, corners, corners, corners, bigCorner);
				buffer.draw_rect_outline(rect, borderColour, 1, corners, corners, corners, bigCorner);

				buffer.draw_rect({rect.x2-(cornerSize+2), rect.y2-(cornerSize+2), rect.x2, rect.y2}, stateColour, bigCorner);
				if(toggleState||isDown){
					buffer.draw_line(rect.x2-cornerSize, rect.y2-1, rect.x2-1, rect.y2-cornerSize, 0x44000000);
				}

				auto fontSettings = graphics2d::Buffer::FontSettings{
					.font=*graphics2d::font::default_sans,
					.size=14,
					.lineSpacing=-4
				};

				auto textSize = buffer.measure_text(fontSettings, text, innerRect.width()-padding-padding);
				auto textWidth = min(textSize.blockWidth, innerRect.width()-padding-padding);
				auto textHeight = min(textSize.blockHeight, innerRect.height()-padding-padding);

				if(icon){
					const auto iconSize = icon->height;
					const auto iconOpacity = opacity*min(255u, (max(6u, iconSize)-6)*255/4)/255 * (isHover?255:77)/255;
					buffer.draw_buffer_blended(innerRect.x1+innerRect.width()/2-icon->width/2, innerRect.y1+innerRect.height()/2-icon->height/2+(isDown?1:0), 0, 0, icon->width, icon->height, *icon, iconOpacity);
				}

				if(toggleState){
					// buffer.draw_line(innerRect.x1+innerRect.width()/2-textWidth/2, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0) + 2, innerRect.x1+innerRect.width()/2-textWidth/2 + textWidth, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0) + 2, graphics2d::blend_colours(bgColour, textColour, 0x88));
					// buffer.draw_rect(innerRect.x1+innerRect.width()/2-textWidth/2-4, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0)-textHeight-4, textWidth+4+4, textHeight+4+4, stateColour);
					// buffer.draw_line(innerRect.x1+innerRect.width()/2-textWidth/2, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0) + 2, innerRect.x1+innerRect.width()/2-textWidth/2 + textWidth, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0) + 2, stateColour);
				}

				if(toggleState){
					buffer.draw_text(fontSettings, text, innerRect.x1+innerRect.width()/2-textWidth/2+1, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0)+1, textWidth, 0xdddddd);
				}
				buffer.draw_text(fontSettings, text, innerRect.x1+innerRect.width()/2-textWidth/2, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0), textWidth, textColour);
			}

			if(false){
				if(rect.width()<6||rect.height()<6) return;

				auto bgColour = isDown?colour:graphics2d::blend_colours(colour, graphics2d::premultiply_colour(0xbbffffff));
				auto borderColour = graphics2d::blend_colours(bgColour, isHover?isDown?0xaa000000:0x82000000:0xbb000000);
				auto textColour = isHover?0x222222:toggleState?0x444444:graphics2d::premultiply_colour(0x66444444);
				auto stateColour = toggleState?0x444444:0x666666;
				auto padding = 2;

				bgColour = graphics2d::blend_colours(backgroundColour, bgColour, opacity);
				borderColour = graphics2d::blend_colours(backgroundColour, borderColour, opacity);
				textColour = graphics2d::blend_colours(bgColour, textColour, opacity);

				U32 corners[3];
				graphics2d::create_diagonal_corner(2, corners);

				auto innerRect = rect.cropped(1,1,1,1);

				buffer.draw_rect(rect, bgColour, corners, corners, corners, corners);
				buffer.draw_rect_outline(rect, borderColour, 1, corners, corners, corners, corners);

				if(toggleState){
					buffer.draw_rect({rect.x1, rect.y2-3, rect.x2, rect.y2}, stateColour, nullptr, nullptr, corners, corners);
				}

				auto fontSettings = graphics2d::Buffer::FontSettings{
					.font=*graphics2d::font::default_sans,
					.size=14,
					.lineSpacing=-4
				};

				auto textSize = buffer.measure_text(fontSettings, text);
				auto textWidth = min(textSize.blockWidth, innerRect.width()-padding-padding);
				auto textHeight = min(textSize.blockHeight, innerRect.height()-padding-padding);

				if(toggleState){
					buffer.draw_text(fontSettings, text, innerRect.x1+innerRect.width()/2-textWidth/2+1, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0)+1, textWidth, 0xdddddd);
				}
				buffer.draw_text(fontSettings, text, innerRect.x1+innerRect.width()/2-textWidth/2, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0), textWidth, textColour);
			}
		}

		void Clean::draw_toggle_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, const char *text, graphics2d::Buffer *icon, bool toggleState, bool isHover, bool isDown) {
			return draw_coloured_toggle_button(buffer, rect, windowBackgroundColour, 0xdddddd, 0xff, text, icon, toggleState, isHover, isDown);
		}
	}
}