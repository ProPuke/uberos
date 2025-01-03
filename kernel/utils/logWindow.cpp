#include "logWindow.hpp"

#include <common/graphics2d/Rect.hpp>
#include <common/graphics2d/font.hpp>
#include <common/stdlib.hpp>

#include <kernel/console.hpp>
#include <kernel/framebuffer.hpp>
#include <kernel/graphics2d.hpp>
#include <kernel/logging.hpp>
#include <kernel/memory.hpp>

namespace utils {
	namespace logWindow {
		namespace {
			graphics2d::View *view = nullptr;
			logging::Handler *logHandler = nullptr;

			auto fontSize = 16;
			auto lineHeight = fontSize*5/4;
			auto leftMargin = 4;
			auto cursorX = leftMargin;
			auto cursorY = lineHeight;
			auto textColour = 0xffffff;
			auto bgColour = 0x202080;

			graphics2d::Rect dirtyArea;

			void print_text(const char *text) {
				//skip escape sequences (only supports the start of strings for now)
				while(text[0]=='\x1b'){
					while(*text&&*text!='m') text++;
					if(*text=='m') text++;
				}

				auto &buffer = view->buffer;
				auto textResult = buffer.draw_text(*graphics2d::font::default_sans, text, leftMargin, cursorY, fontSize, textColour, lineHeight, cursorX);
				if(max(cursorY, textResult.updatedArea.y2)>=(I32)buffer.height){
					auto scroll = lineHeight*8;
					buffer.scroll(0, -scroll);
					// buffer.draw_rect(0, buffer.height-scroll, buffer.width, scroll, bgColour);
					cursorY -= scroll;

					//redraw as it was clipped off last time
					buffer.draw_rect(0, cursorY-lineHeight, buffer.width, buffer.height-(cursorY-lineHeight), bgColour); //clear the old text as well as the bottom first (to avoid bolding from double text drawing) ...
					textResult = buffer.draw_text(*graphics2d::font::default_sans, text, leftMargin, cursorY, fontSize, textColour, lineHeight, cursorX); //...then redraw the text
					graphics2d::update_view(*view);
					dirtyArea = {0,0,0,0};

				}else{
					if(dirtyArea.isNonzero()){
						dirtyArea.include(textResult.updatedArea);
					}else{
						dirtyArea = textResult.updatedArea;
					}
				}
				cursorX = textResult.x;
				cursorY = textResult.y;
			}
		}

		void install() {
			if(view) return;

			auto margin = 20;

			auto &framebuffer = *framebuffer::get_framebuffer(0); //FIXME: handle 0 framebuffers

			view = graphics2d::create_view(nullptr, graphics2d::ViewLayer::topMost, margin, margin, min(1300u, framebuffer.buffer.width-margin*2), min(800u, framebuffer.buffer.height-margin*2));
			// view = graphics2d::create_view(nullptr, graphics2d::ViewLayer::topMost, margin, margin, min(1300u, framebuffer.buffer.width-margin*2), 256);

			view->buffer.draw_rect(0, 0, view->buffer.width, view->buffer.height, bgColour);

			graphics2d::update_view(*view);

			logHandler = new logging::Handler(
				[](U32 indent, logging::PrintType type) {
					while(indent--) print_text("  ");
					switch(type){
						case logging::PrintType::info:
							textColour = 0xffffff;
						break;
						case logging::PrintType::debug:
							textColour = 0xffff00;
						break;
						case logging::PrintType::warning:
							textColour = 0xff8000;
						break;
						case logging::PrintType::error:
							textColour = 0xff0000;
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
					graphics2d::update_view_area(*view, dirtyArea);
					dirtyArea = {0,0,0,0};
				}
			);

			logging::install_handler(*logHandler);
		}

		void show() {
			graphics2d::show_view(*view);
		}

		void hide() {
			graphics2d::hide_view(*view);
		}
	}
}
