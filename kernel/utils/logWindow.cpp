#include "logWindow.hpp"

#include <drivers/DesktopManager.hpp>

#include <kernel/console.hpp>
#include <kernel/DriverReference.hpp>
#include <kernel/drivers.hpp>
#include <kernel/logging.hpp>
#include <kernel/memory.hpp>

#include <common/graphics2d/Rect.hpp>
#include <common/graphics2d/font.hpp>
#include <common/stdlib.hpp>

namespace utils {
	namespace logWindow {
		namespace {
			driver::DesktopManager::StandardWindow *window = nullptr;
			logging::Handler *logHandler = nullptr;

			auto fontSize = 10u;
			auto lineHeight = (U32)(graphics2d::font::default_console->lineHeight * fontSize + 0.5);
			auto leftMargin = 4;
			auto cursorX = leftMargin;
			auto cursorY = (I32)lineHeight;
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

			DriverReference<driver::DesktopManager> desktopManager{nullptr, [](void*){
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

				auto &clientArea = window->get_client_area();

				auto fontSettings = graphics2d::Buffer::FontSettings{
					.font = *graphics2d::font::default_console,
					.size = fontSize
				};

				auto textResult = clientArea.draw_text(fontSettings, text, leftMargin + cursorColumn * columnWidth, cursorY, columnWidth, textColour, cursorX);
				if(max(cursorY, textResult.updatedArea.y2)>=(I32)clientArea.height){
					if(columns==1){
						// if a single column, scroll

						// auto scroll = lineHeight*8;
						auto scroll = lineHeight*4;
						clientArea.scroll(0, -scroll);
						cursorY -= scroll;

						//redraw as it was clipped off last time
						clientArea.draw_rect(0, clientArea.height-scroll, clientArea.width, scroll, window->get_background_colour()); //clear the bottom section..
						textResult = clientArea.draw_text(fontSettings, text, leftMargin + cursorColumn * columnWidth, cursorY, columnWidth, textColour, cursorX); //...then redraw the text
						dirtyArea.include({0, 0, (I32)clientArea.width, (I32)clientArea.height});

					}else{
						// if multi-column, change to the next column (cyclically) and clear the space for new content

						// erase the ENTIRE previous print, as we are now movig it into the next column
						clientArea.draw_rect(textResult.updatedArea.x1, textResult.updatedArea.y1, textResult.updatedArea.x2-textResult.updatedArea.x1, textResult.updatedArea.y2-textResult.updatedArea.y1, window->get_background_colour());

						cursorColumn = (cursorColumn+1)%columns;

						cursorX = leftMargin + cursorColumn * columnWidth;
						cursorY = lineHeight;

						clientArea.draw_rect(cursorColumn * columnWidth, 0, columnWidth, clientArea.height, window->get_background_colour());
						dirtyArea = dirtyArea.include({cursorColumn * columnWidth, 0, cursorColumn * columnWidth + columnWidth, (I32)clientArea.height});

						textResult = clientArea.draw_text(fontSettings, text, leftMargin + cursorColumn * columnWidth, cursorY, columnWidth, textColour, cursorX);
					}

				}else{
					dirtyArea = dirtyArea.include(textResult.updatedArea);
				}

				cursorX = textResult.x;
				cursorY = textResult.y;
			}

			void redraw() {
				cursorX = leftMargin;
				cursorY = lineHeight;
				cursorColumn = 0;
				auto &clientArea = window->get_client_area();
				columns = max(1u, clientArea.width / 500u);
				columnWidth = clientArea.width / columns;

				textColour = textColourHistory;

				print_multiline_text(logging::get_history_part_1());
				print_multiline_text(logging::get_history_part_2());

				textColour = textColourInfo;
			}

			void on_window_event(const driver::DesktopManager::Window::Event &event) {
				if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
					redraw();
				}
			}
		}

		void install() {
			if(window) return;

			auto margin = 20;

			desktopManager = drivers::find_and_activate<driver::DesktopManager>();
			if(!desktopManager) return;

			auto windowArea = desktopManager->get_window_area();

			auto viewWidth = (U32)min(1600, windowArea.width()-margin*2);
			auto viewHeight = (U32)min(1000, windowArea.height()-margin*2);

			// TODO:topmost?
			window = &desktopManager->create_standard_window("Kernel Log", viewWidth, viewHeight);
			window->events.subscribe(on_window_event);
			// view = graphics2d::create_view(nullptr, graphics2d::DisplayLayer::topMost, margin, margin, min(1300u, framebuffer.clientArea.width-margin*2), 256);

			// window->set_status("Booting...");

			redraw();

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

			window->show();
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
