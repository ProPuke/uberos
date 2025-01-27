#include "logWindow.hpp"

#include <common/graphics2d/Rect.hpp>
#include <common/graphics2d/font.hpp>
#include <common/stdlib.hpp>

#include <kernel/console.hpp>
#include <kernel/drivers/common/system/DesktopManager.hpp>
#include <kernel/framebuffer.hpp>
#include <kernel/logging.hpp>
#include <kernel/memory.hpp>

namespace utils {
	namespace logWindow {
		namespace {
			driver::system::DesktopManager::Window *window = nullptr;
			logging::Handler *logHandler = nullptr;

			auto fontSize = 10;
			auto lineHeight = fontSize*5/4;
			auto leftMargin = 4;
			auto cursorX = leftMargin;
			auto cursorY = lineHeight;
			auto cursorColumn = 0;
			auto columns = 1;
			auto columnWidth = 1000;
			// auto textColourInfo = 0xffffff;
			auto textColourHistory = 0x888888;
			auto textColourInfo = 0x222222;
			auto textColourDebug = 0x008888;
			auto textColourWarning = 0xff8000;
			auto textColourError = 0xff0000;
			auto textColour = textColourInfo;

			graphics2d::Rect dirtyArea;

			DriverReference<driver::system::DesktopManager> desktopManager{nullptr, [](void*){
				if(window){
					//TODO:free this?
					window = nullptr;
				}
			}, nullptr};

			void print_text(const char *text);

			void print_multiline_text(const char *text) {
				while(auto lineEnd = strchr(text, '\n')){
					static char line[256];
					auto length = min((size_t)(lineEnd+1-text), sizeof(line)-1);
					memcpy(line, text, length);
					line[length] = '\0';
					print_text(line);
					text += length;
				}
				print_text(text);
			}

			// NOTE: this will wrap the entire text element when it exceeds the bottom. This means if feeding it multiple lines, you are best chunking them by individual line, first
			void print_text(const char *text) {
				//skip escape sequences (only supports the start of strings for now)
				while(text[0]=='\x1b'){
					while(*text&&*text!='m') text++;
					if(*text=='m') text++;
				}

				auto &buffer = window->clientArea;

				auto textResult = buffer.draw_text(*graphics2d::font::default_console, text, leftMargin + cursorColumn * columnWidth, cursorY, columnWidth, fontSize, textColour, lineHeight, cursorX);
				if(max(cursorY, textResult.updatedArea.y2)>=(I32)buffer.height){

					if(columns==1){
						// if a single column, scroll

						// auto scroll = lineHeight*8;
						auto scroll = lineHeight*4;
						buffer.scroll(0, -scroll);
						// buffer.draw_rect(0, buffer.height-scroll, buffer.width, scroll, window->get_background_colour());
						cursorY -= scroll;

						//redraw as it was clipped off last time
						buffer.draw_rect(0, buffer.height-scroll, buffer.width, scroll, window->get_background_colour()); //clear the bottom section..
						textResult = buffer.draw_text(*graphics2d::font::default_console, text, leftMargin + cursorColumn * columnWidth, cursorY, columnWidth, fontSize, textColour, lineHeight, cursorX); //...then redraw the text
						window->redraw();
						dirtyArea = {0,0,0,0};

					}else{
						// if multi-column, change to the next column (cyclically) and clear the space for new content

						// erase the ENTIRE previous print, as we are now movig it into the next column
						buffer.draw_rect(textResult.updatedArea.x1, textResult.updatedArea.y1, textResult.updatedArea.x2-textResult.updatedArea.x1, textResult.updatedArea.y2-textResult.updatedArea.y1, window->get_background_colour());

						cursorColumn = (cursorColumn+1)%columns;

						cursorX = leftMargin + cursorColumn * columnWidth;
						cursorY = lineHeight;

						buffer.draw_rect(cursorColumn * columnWidth, 0, columnWidth, buffer.height, window->get_background_colour());
						dirtyArea = dirtyArea.include({cursorColumn * columnWidth, 0, cursorColumn * columnWidth + columnWidth, (I32)buffer.height});

						textResult = buffer.draw_text(*graphics2d::font::default_console, text, leftMargin + cursorColumn * columnWidth, cursorY, columnWidth, fontSize, textColour, lineHeight, cursorX);
					}

				}else{
					dirtyArea = dirtyArea.include(textResult.updatedArea);
				}

				cursorX = textResult.x;
				cursorY = textResult.y;
			}
		}

		void install() {
			if(window) return;

			auto margin = 20;

			desktopManager = drivers::find_and_activate<driver::system::DesktopManager>();
			if(!desktopManager) return;

			if(framebuffer::get_framebuffer_count()<1) return;

			auto &framebuffer = *framebuffer::get_framebuffer(0);

			auto viewWidth = min(1600u, framebuffer.buffer.width-margin*2);
			auto viewHeight = min(1000u, framebuffer.buffer.height-margin*2);

			// TODO:topmost?
			window = &desktopManager->create_window("Kernel Log", viewWidth, viewHeight);
			// view = graphics2d::create_view(nullptr, graphics2d::DisplayLayer::topMost, margin, margin, min(1300u, framebuffer.buffer.width-margin*2), 256);

			window->clientArea.draw_rect(0, 0, window->clientArea.width, window->clientArea.height, window->get_background_colour());

			// window->set_status("Booting...");

			columns = max(1u, window->clientArea.width / 500u);
			columnWidth = window->clientArea.width / columns;

			window->redraw();

			textColour = textColourHistory;

			print_multiline_text(logging::get_history_part_1());
			print_multiline_text(logging::get_history_part_2());

			textColour = textColourInfo;

			logHandler = new logging::Handler(
				[](U32 indent, logging::PrintType type) {
					while(indent--) print_text("  ");
					switch(type){
						case logging::PrintType::info:
							textColour = textColourInfo;
						break;
						case logging::PrintType::debug:
							textColour = textColourDebug;
						break;
						case logging::PrintType::warning:
							textColour = textColourWarning;
						break;
						case logging::PrintType::error:
							textColour = textColourError;
						break;
					}
				},
				[](char c) {
					char str[2] = {c, '\0'};
					print_text(str);
				},
				[](const char *str) {
					print_text(str);
				},
				[]() {
					print_text("\n");
					window->redraw_area(dirtyArea);
					dirtyArea = {0,0,0,0};
				}
			);

			logging::install_handler(*logHandler);
		}

		void show() {
			if(!window) return;
			window->show();
		}

		void hide() {
			if(!window) return;
			window->hide();
		}
	}
}
