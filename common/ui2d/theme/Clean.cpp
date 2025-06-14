#include "Clean.hpp"

#include <kernel/memory.hpp>

#include <common/graphics2d/font.hpp>

namespace ui2d {
	namespace theme {
		namespace {
			const auto enableTransparency = true;

			namespace window {
				const U32 backgroundColour = 0xeeeeee;
				const U32 borderColour = 0xbcbcbc;

				const auto titlebarHeight = 30u;
				const auto statusbarHeight = 23u;

				const auto cornerRadius = enableTransparency?5:2;
				U32 corner[cornerRadius+1];
				U32 cornerInner[cornerRadius-1+1];

				const U8 corner5x5Graphic[5*5] = "\000\000!\231\342\000G\353\202'!\353=\000\000\231\203\000\000\000\342(\000\000";
			}
		}

		/**/ Clean::Clean() {
			leftShadow = enableTransparency?8:0;
			rightShadow = enableTransparency?8:0;
			topShadow = enableTransparency?8:0;
			bottomShadow = enableTransparency?8:0;

			graphics2d::create_diagonal_corner(window::cornerRadius, window::corner);
			graphics2d::create_diagonal_corner(window::cornerRadius-1, window::cornerInner);
		}

		auto Clean::get_component_spacing() -> I32 { return 7; }
		auto Clean::get_section_spacing() -> I32 { return 11; }

		auto Clean::get_minimum_button_width() -> I32 {
			return 100;
		}
		auto Clean::get_minimum_button_height() -> I32 {
			return 28;
		}

		void Clean::draw_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool isHover, bool isDown) {
			return draw_coloured_button(buffer, rect, window::backgroundColour, 0xdddddd, 0xff, text, smallFont, icon, isHover, isDown);
		}

		void Clean::draw_coloured_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool isHover, bool isDown) {
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
				.size=smallFont?12u:14u,
				.lineSpacing=-4
			};

			auto textSize = buffer.measure_text(fontSettings, text);
			auto textWidth = min(textSize.rect.width(), innerRect.width()-padding-padding);
			auto textHeight = min(textSize.rect.height(), innerRect.height()-padding-padding);

			if(auto iconBuffer = icon.get_size_or_larger_or_smaller(innerRect.height())){
				const auto iconSize = iconBuffer->height;
				const auto iconOpacity = opacity*min(255u, (max(6u, iconSize)-6)*255/4)/255 * (isHover?255:77)/255;
				buffer.draw_buffer_blended(innerRect.x1+innerRect.width()/2-iconBuffer->width/2, innerRect.y1+innerRect.height()/2-iconBuffer->height/2+(isDown?1:0), 0, 0, iconBuffer->width, iconBuffer->height, *iconBuffer, iconOpacity);
			}

			buffer.draw_text(fontSettings, text, innerRect.x1+innerRect.width()/2-textWidth/2, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0), textWidth, textColour);
		}

		void Clean::draw_coloured_toggle_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, U32 backgroundColour, U32 colour, U8 opacity, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool toggleState, bool isHover, bool isDown) {
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
					.size=smallFont?12u:14u,
					.lineSpacing=-4
				};

				auto textSize = buffer.measure_text(fontSettings, text, innerRect.width()-padding-padding);
				auto textWidth = min(textSize.rect.width(), innerRect.width()-padding-padding);
				auto textHeight = min(textSize.rect.height(), innerRect.height()-padding-padding);

				if(icon){
					auto iconBuffer = icon.get_size_or_larger_or_smaller(innerRect.height());
					const auto iconSize = iconBuffer->height;
					const auto iconOpacity = opacity*min(255u, (max(6u, iconSize)-6)*255/4)/255 * (isHover?255:77)/255;
					buffer.draw_buffer_blended(innerRect.x1+innerRect.width()/2-iconBuffer->width/2, innerRect.y1+innerRect.height()/2-iconBuffer->height/2+(isDown?1:0), 0, 0, iconBuffer->width, iconBuffer->height, *iconBuffer, iconOpacity);
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
					.size=smallFont?12u:14u,
					.lineSpacing=-4
				};

				auto textSize = buffer.measure_text(fontSettings, text);
				auto textWidth = min(textSize.rect.width(), innerRect.width()-padding-padding);
				auto textHeight = min(textSize.rect.height(), innerRect.height()-padding-padding);

				if(toggleState){
					buffer.draw_text(fontSettings, text, innerRect.x1+innerRect.width()/2-textWidth/2+1, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0)+1, textWidth, 0xdddddd);
				}
				buffer.draw_text(fontSettings, text, innerRect.x1+innerRect.width()/2-textWidth/2, innerRect.y1+innerRect.height()/2+textSize.capHeight-textHeight/2+(isDown?1:0), textWidth, textColour);
			}
		}

		void Clean::draw_toggle_button(graphics2d::Buffer &buffer, graphics2d::Rect rect, const char *text, bool smallFont, graphics2d::MultisizeIcon icon, bool toggleState, bool isHover, bool isDown) {
			return draw_coloured_toggle_button(buffer, rect, window::backgroundColour, 0xdddddd, 0xff, text, smallFont, icon, toggleState, isHover, isDown);
		}

		auto Clean::get_window_min_width() -> U32 {
			return window::titlebarHeight*5;
		}
		auto Clean::get_window_min_height() -> U32 {
			return window::titlebarHeight+window::statusbarHeight-1;
		}

		auto Clean::get_window_titlebar_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return {(I32)leftShadow, (I32)topShadow, rect.x2-(I32)rightShadow, (I32)topShadow+(I32)window::titlebarHeight};
		}

		auto Clean::get_window_titlebar_divider_colour(WindowFrameOptions options) -> U32 {
			const auto titlebarBgColour = /*draggingCursor?0xfff9f9:*/options.isFocused?0xf9f9f9:window::backgroundColour;
			return graphics2d::blend_colours(titlebarBgColour, options.isFocused?0xdd000000:0xee000000);
		}

		auto Clean::get_window_titlebar_widget_margin() -> U32 {
			return 4;
		}

		auto Clean::get_window_titlebar_widget_spacing() -> U32 {
			return 4;
		}

		auto Clean::get_window_titlebar_widget_large_spacing() -> U32 {
			return 4*2;
		}

		auto Clean::has_window_titlebar_widget_coloured() -> bool {
			return true;
		}

		auto Clean::get_window_statusbar_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return {rect.x1+(I32)leftShadow+1, rect.y2-(I32)bottomShadow-(I32)window::statusbarHeight+1, rect.x2-(I32)rightShadow-1, rect.y2-(I32)bottomShadow-1};
			// return {rect.x1+(I32)leftShadow+1, rect.y2-(I32)bottomShadow-(I32)window::statusbarHeight+1, borderArea.width()-2, window::statusbarHeight-2};
		}

		auto Clean::get_window_client_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return rect.cropped((I32)leftShadow+1, (I32)topShadow+window::titlebarHeight, (I32)rightShadow+1, (I32)bottomShadow+23);
		}

		auto Clean::get_window_solid_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return rect.cropped(leftShadow+window::cornerRadius, topShadow, rightShadow+window::cornerRadius, bottomShadow);
		}

		auto Clean::get_window_interact_area(graphics2d::Rect rect) -> graphics2d::Rect {
			return rect.cropped(leftShadow, topShadow, rightShadow, bottomShadow);
		}

		auto Clean::get_window_background_colour() -> U32 {
			return 0xeeeeee;
		}

		void Clean::draw_window_frame(graphics2d::Buffer &_buffer, graphics2d::Rect rect, WindowFrameOptions options) {
			graphics2d::Buffer buffer = _buffer.region(rect.x1, rect.y1, rect.width(), rect.height());
			rect = rect.offset(-rect.x1, -rect.y1);

			auto borderArea = get_window_interact_area(rect);
			// auto clientArea = get_window_client_area(rect);
			auto titlebarArea = get_window_titlebar_area(rect);
			const auto borderColour = /*draggingCursor?0xd0b0b0:*/window::borderColour;
			const auto titlebarBgColour = /*draggingCursor?0xfff9f9:*/options.isFocused?0xf9f9f9:window::backgroundColour;
			const auto titlebarTextColour = options.isFocused?0x333333:0x999999;
			const auto statusbarTextColour = options.isFocused?0x666666:0xaaaaaa;

			U32 innerAaCorner[2+1];
			graphics2d::create_diagonal_corner(2, innerAaCorner);

			if(enableTransparency){
				{ // draw border minus corners
					buffer.draw_line(borderArea.x1+window::cornerRadius, borderArea.y1, borderArea.x2-1-window::cornerRadius, borderArea.y1, borderColour);
					buffer.draw_line(borderArea.x1+window::cornerRadius, borderArea.y2-1, borderArea.x2-1-window::cornerRadius, borderArea.y2-1, borderColour);
					buffer.draw_line(borderArea.x1, borderArea.y1+window::cornerRadius, borderArea.x1, borderArea.y2-window::cornerRadius, borderColour);
					buffer.draw_line(borderArea.x2-1, borderArea.y1+window::cornerRadius, borderArea.x2-1, borderArea.y2-window::cornerRadius, borderColour);
				}

			}else{
				buffer.draw_rect_outline(borderArea.x1, borderArea.y1, rect.width(), rect.height(), borderColour, 1, window::corner, window::corner, window::corner, window::corner);
				buffer.draw_rect(borderArea.x1+1, borderArea.y1+1, rect.width()-2, window::titlebarHeight-2, titlebarBgColour, window::corner, window::corner, nullptr, nullptr);
			}

			{ // draw titlebar divide
				buffer.draw_line(borderArea.x1+1, borderArea.y1+window::titlebarHeight-1, borderArea.x2-2, borderArea.y1+window::titlebarHeight-1, options.isFocused?borderColour:window::backgroundColour);
			}

			{ // draw statusbar divide
				buffer.draw_line(borderArea.x1+1, borderArea.y1+borderArea.height()-1-21-1, borderArea.x2-2, borderArea.y1+borderArea.height()-window::statusbarHeight, options.isFocused?0xcccccc:0xdddddd);
			}

			if(enableTransparency){
				{ // undraw any previous corners with transparency, otherwise we get line doubling on the aa pixels
					buffer.draw_rect(borderArea.x1, borderArea.y1, window::cornerRadius, window::cornerRadius, 0xff000000);
					buffer.draw_rect(borderArea.x2-window::cornerRadius, borderArea.y1, window::cornerRadius, window::cornerRadius, 0xff000000);
					buffer.draw_rect(borderArea.x1, borderArea.y2-window::cornerRadius, window::cornerRadius, window::cornerRadius, 0xff000000);
					buffer.draw_rect(borderArea.x2-window::cornerRadius, borderArea.y2-window::cornerRadius, window::cornerRadius, window::cornerRadius, 0xff000000);

					// put shadows under corners
					for(auto y=0;y<window::cornerRadius;y++){
						auto width = 1+window::cornerRadius-1-y;
						for(auto x=0;x<width;x++){
							buffer.set(borderArea.x1+x, borderArea.y1+y, 0x000000|(255-get_shadow_intensity_at(rect, borderArea.x1+x, borderArea.y1+y)<<24));
							buffer.set(borderArea.x2-1-x, borderArea.y1+y, 0x000000|(255-get_shadow_intensity_at(rect, borderArea.x2-1-x, borderArea.y1+y)<<24));
							buffer.set(borderArea.x1+x, borderArea.y2-1-y, 0x000000|(255-get_shadow_intensity_at(rect, borderArea.x1+x, borderArea.y2-1-y)<<24));
							buffer.set(borderArea.x2-1-x, borderArea.y2-1-y, 0x000000|(255-get_shadow_intensity_at(rect, borderArea.x2-1-x, borderArea.y2-1-y)<<24));
						}
					}
				}

				{ // draw titlebar block
					if(options.isFocused){
						buffer.draw_rect(borderArea.x1+1, borderArea.y1+1, borderArea.width()-2, window::titlebarHeight-2, titlebarBgColour, innerAaCorner, innerAaCorner, nullptr, nullptr);
					}else{
						buffer.draw_rect(borderArea.x1+1, borderArea.y1+1, borderArea.width()-2, window::titlebarHeight-2, titlebarBgColour, innerAaCorner, innerAaCorner, nullptr, nullptr);
						// for(auto y=0;y<titlebarHeight-2;y++){
						// 	buffer.set(borderArea.x1+1+graphicsDisplay->get_left_margin(borderArea.y1+1+y), (U32)borderArea.y1+1+y, graphics2d::blend_rgb(0xf9f9f9, titlebarBgColour, min(1.0f, y/(float)(titlebarHeight/4-3))), rect.width()-2-graphicsDisplay->get_left_margin(borderArea.y1+1+y)-graphicsDisplay->get_right_margin(borderArea.y1+1+y));
						// }
					}
				}

				{ // draw statusbar block
					const auto statusbarArea = get_window_statusbar_area(rect);
					buffer.draw_rect(statusbarArea, window::backgroundColour, nullptr, nullptr, innerAaCorner, innerAaCorner);
				}

				{ // draw aa corners
					for(auto y=0;y<5;y++){
						for(auto x=0;x<5;x++){
							buffer.set_blended(borderArea.x1+x, borderArea.y1+y, graphics2d::premultiply_colour((borderColour&0x00ffffff)|((255-window::corner5x5Graphic[y*5+x])<<24)));
							buffer.set_blended(borderArea.x2-1-x, borderArea.y1+y, graphics2d::premultiply_colour((borderColour&0x00ffffff)|((255-window::corner5x5Graphic[y*5+x])<<24)));
							buffer.set_blended(borderArea.x1+x, borderArea.y2-1-y, graphics2d::premultiply_colour((borderColour&0x00ffffff)|((255-window::corner5x5Graphic[y*5+x])<<24)));
							buffer.set_blended(borderArea.x2-1-x, borderArea.y2-1-y, graphics2d::premultiply_colour((borderColour&0x00ffffff)|((255-window::corner5x5Graphic[y*5+x])<<24)));
						}
					}
				}

			}else{
				{ // draw titlebar block
					buffer.draw_rect(borderArea.x1+1, borderArea.y1+1, borderArea.width()-2, window::titlebarHeight-2, titlebarBgColour, window::cornerInner, window::cornerInner, nullptr, nullptr);
				}

				{ // draw statusbar block
					const auto statusbarArea = get_window_statusbar_area(rect);
					buffer.draw_rect(statusbarArea, window::backgroundColour, nullptr, nullptr, window::cornerInner, window::cornerInner);
				}
			}

			{ // draw titlebar text
				const auto centredIndent = (I32)maths::max(options.titlebarAreaIndentLeft, options.titlebarAreaIndentRight); // matching indent for both sides, to centre
				const auto fullWidth = titlebarArea.width()-options.titlebarAreaIndentLeft-options.titlebarAreaIndentRight; // the full width, from left to right widgets
				const auto centredWidth = titlebarArea.width()-centredIndent*2; // a smaller width that sits comfortably in the centre (so same indent on both sides)
				auto fontSettings = graphics2d::Buffer::FontSettings{.font=*graphics2d::font::default_sans, .size=14, .clipped=true, .maxLines=1};
				
				auto textSize = buffer.measure_text(fontSettings, options.title, fullWidth);
				while(textSize.clipped&&fontSettings.size>12){
					fontSettings.size--;
					textSize = buffer.measure_text(fontSettings, options.title, fullWidth);
				}

				if(!textSize.clipped&&textSize.rect.width()<=centredWidth){ // fits in the centre column
					buffer.draw_text(fontSettings, options.title, titlebarArea.x1+centredIndent+(centredWidth-textSize.rect.width())/2, titlebarArea.y1+(window::titlebarHeight-2)/2+textSize.capHeight-textSize.rect.height()/2, centredWidth, titlebarTextColour);

				}else if(textSize.clipped){ // doesn't even fit in the full width
					buffer.draw_text(fontSettings, options.title, titlebarArea.x1+options.titlebarAreaIndentLeft+(fullWidth-textSize.rect.width())/2, titlebarArea.y1+(window::titlebarHeight-2)/2+textSize.capHeight-textSize.rect.height()/2, fullWidth, titlebarTextColour);

				}else{ // fits in the full width, but exceeds the centre column, so left or right align within the area
					if(options.titlebarAreaIndentRight>options.titlebarAreaIndentLeft){
						// sit against right
						buffer.draw_text(fontSettings, options.title, titlebarArea.x2-options.titlebarAreaIndentRight-textSize.rect.width(), titlebarArea.y1+(window::titlebarHeight-2)/2+textSize.capHeight-textSize.rect.height()/2, fullWidth, titlebarTextColour);
					}else{
						// sit against left
						buffer.draw_text(fontSettings, options.title, titlebarArea.x1+options.titlebarAreaIndentLeft, titlebarArea.y1+(window::titlebarHeight-2)/2+textSize.capHeight-textSize.rect.height()/2, fullWidth, titlebarTextColour);
					}
				}
			}

			{ // draw statusbar text
				// auto lineHeight = 14*5/4;
				buffer.draw_text({.font=*graphics2d::font::default_sans, .size=14}, options.status, borderArea.x1+4, borderArea.y2-7, borderArea.width(), statusbarTextColour);
			}

			if(enableTransparency){ // draw shadow
				for(auto y=0u; y<buffer.height; y++){
					for(auto x=0u; x<leftShadow; x++){
						buffer.set(x, y, 0x000000|(255-get_shadow_intensity_at(rect, x, y)<<24));
					}
					for(auto x=0u; x<rightShadow; x++){
						buffer.set(leftShadow+borderArea.width()+rightShadow-1-x, y, 0x000000|(255-get_shadow_intensity_at(rect, x, y)<<24));
					}
				}

				for(auto y=0u; y<topShadow; y++){
					for(auto x=0u;x<(U32)borderArea.width();x++){
						buffer.set(leftShadow+x, y, 0x000000|(255-get_shadow_intensity_at(rect, leftShadow+x, y)<<24));
					}
				}

				for(auto y=0u; y<bottomShadow; y++){
					// buffer.set(leftShadow, borderArea.height()+y, 0x000000|(255-get_shadow_intensity_at(rect, leftShadow, borderArea.height()+y)<<24), borderArea.width());

					for(auto x=0u;x<(U32)borderArea.width();x++){
						buffer.set(leftShadow+x, topShadow+borderArea.height()+y, 0x000000|(255-get_shadow_intensity_at(rect, leftShadow+x, (I32)topShadow+borderArea.height()+y)<<24));
					}
				}

				// shadowDisplay = displayManager->create_display(nullptr, DisplayManager::DisplayLayer::regular, graphicsDisplay->x+5, graphicsDisplay->x+5, graphicsDisplay->get_width(), graphicsDisplay->get_height());
				// // shadowDisplay->mode = DisplayManager::DisplayMode::transparent;
				// shadowDisplay->solidArea.clear();
				// shadowDisplay->isDecoration = true;
				// shadowDisplay->buffer.draw_rect(0, 0, shadowDisplay->get_width(), shadowDisplay->get_height(), 0x80000000, corner, corner, corner, corner);
				// shadowDisplay->place_below(*graphicsDisplay);
				// shadowDisplay->update();
			}
		}

		auto Clean::get_box_client_area(graphics2d::Rect rect, BoxType boxType) -> graphics2d::Rect {
			auto spacing = get_component_spacing();
			return rect.cropped(2+spacing, 2+spacing, 2+spacing, 2+spacing);
		}

		void Clean::draw_box(graphics2d::Buffer &buffer, graphics2d::Rect rect, BoxType boxType) {
			U32 corners[3];
			graphics2d::create_diagonal_corner(2, corners);

			switch(boxType){
				case BoxType::default_:
					buffer.draw_rect_outline(rect, 0xcccccc, 1, corners, corners, corners, corners);
				break;
				case BoxType::inset:
					//TODO: inset border?
					buffer.draw_rect_outline(rect, 0xcccccc, 1, corners, corners, corners, corners);
				break;
			}
		}
	}
}