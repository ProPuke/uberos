#include <common/graphics2d/font.hpp>
#include <kernel/drivers.hpp>
#include <kernel/drivers/common/system/DesktopManager.hpp>
#include <kernel/drivers/Keyboard.hpp>
#include <kernel/keyboard.hpp>
#include <kernel/keyboard/layout/uk.hpp>

namespace test {
	auto get_scankey_name(keyboard::Scancode scancode) -> const char * {
		#include <kernel/keyboard/layout/uk.hpp>

		#define KEY(NAME,...) case (keyboard::Scancode)keyboard::ScancodeUk::NAME: return #NAME,##__VA_ARGS__;
		#define EMPTY
		#define EXTEND_UP
		#define EXTEND_DOWN
		#define EXTEND_LEFT
		#define EXTEND_RIGHT

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wunused-value"

		switch(scancode) {
			KEYBOARD_LAYOUT_UK_ROW0
			KEYBOARD_LAYOUT_UK_ROW1
			KEYBOARD_LAYOUT_UK_ROW2
			KEYBOARD_LAYOUT_UK_ROW3
			KEYBOARD_LAYOUT_UK_ROW4
			KEYBOARD_LAYOUT_UK_ROW5
			KEYBOARD_LAYOUT_UK_ROW6
			KEYBOARD_LAYOUT_UK_ROW7
			default:
				return "unknown";
		}

		#pragma GCC diagnostic pop

		#undef KEY
		#undef EMPTY
		#undef EXTEND_UP
		#undef EXTEND_DOWN
		#undef EXTEND_LEFT
		#undef EXTEND_RIGHT
		#undef KEYBOARD_LAYOUT_ROW0
		#undef KEYBOARD_LAYOUT_ROW1
		#undef KEYBOARD_LAYOUT_ROW2
		#undef KEYBOARD_LAYOUT_ROW3
		#undef KEYBOARD_LAYOUT_ROW4
		#undef KEYBOARD_LAYOUT_ROW5
		#undef KEYBOARD_LAYOUT_ROW6
		#undef KEYBOARD_LAYOUT_ROW7
	}

	void start_tasks() {
		if(auto desktopManager = drivers::find_and_activate<driver::system::DesktopManager>()) {
			auto window = &desktopManager->create_window("Font Test", 320, 320);
			// view = graphics2d::create_view(nullptr, graphics2d::DisplayLayer::topMost, margin, margin, min(1300u, framebuffer.buffer.width-margin*2), 256);

			auto y = 32;
			window->clientArea.draw_text(*graphics2d::font::default_sans, "Abc", 10+1, y+1, 320, 38, 0x222222);
			window->clientArea.draw_text(*graphics2d::font::default_sans, "Abc", 10, y, 320, 38, 0xaaaaaa);

			y += 45;
			window->clientArea.draw_text(*graphics2d::font::default_sans, "Abc", 10+1, y+1, 320, 72, 0x222222);
			window->clientArea.draw_text(*graphics2d::font::default_sans, "Abc", 10, y, 320, 72, 0xaaaaaa);

			y += 70;
			window->clientArea.draw_text(*graphics2d::font::default_sans, "Abc", 10+1, y+1, 320, 100, 0x222222);
			window->clientArea.draw_text(*graphics2d::font::default_sans, "Abc", 10, y, 320, 100, 0xaaaaaa);

			y += 98;
			window->clientArea.draw_text(*graphics2d::font::default_sans, "Abc", 10+1, y+1, 320, 150, 0x222222);
			window->clientArea.draw_text(*graphics2d::font::default_sans, "Abc", 10, y, 320, 150, 0xaaaaaa);

			window->redraw();
		}

		if(auto desktopManager = drivers::find_and_activate<driver::system::DesktopManager>()) {
			const static auto padding = 6;

			auto window = &desktopManager->create_window("Keyboard Test", 888+padding*2, 303+padding*2);

			auto buffer = window->clientArea.cropped(padding, padding, padding, padding);

			const auto keySize = 30;

			static bool staggered = true;
			static bool join = true;

			static const auto colX = [=](U32 row, U32 col) -> U32 {
				if(!staggered) return col*keySize;

				if(col>5&&col<20&&row==0){
					return col*keySize+(+keySize*1/3*(14-(col-5))/14)-(keySize*4/8*(col-5)/14);

				}else if(col>5&&col<20&&row==1){
					return col*keySize+keySize*2/8;

				}else if(col>5&&col<20&&row==2){
					return col*keySize-(col>6?keySize*2/8:keySize*1/8);

				}else if(col>5&&col<20&&row==3){
					return col*keySize-(col>6?keySize*1/2:keySize*1/4);
					// return col*keySize-keySize*5/8;

				}else{
					return col*keySize;
				}
			};

			U32 corner[] = {1, (U32)-1};

			#define KEY(NAME,...) \
				if(i==0)buffer.draw_rect_outline(colX(row, col)+1, (keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(i==1)buffer.draw_rect(colX(row, col)+2, (keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col)-4, keySize-4, 0xffffff);\
				if(i==1&&(false,##__VA_ARGS__))buffer.draw_text(*graphics2d::font::default_sans, ("",##__VA_ARGS__), colX(row, col)+1+3, (keyboard::Position::maxRows-1-row)*keySize+3+8, keySize-6, 8, 0x222222);\
				col++;
			#define EMPTY \
				if(i==0)buffer.draw_rect_outline(colX(row, col)+1, (keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xdddddd, 1, corner, corner, corner, corner);\
				col++;
			#define EXTEND_UP \
				if(i==0)buffer.draw_rect_outline(colX(row, col)+1, (keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==1)buffer.draw_rect(colX(row, col)+2, (keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col)-4, keySize-4, 0xffffff);\
				if(join&&i==1)buffer.draw_rect(colX(row+1, col)+2, (keyboard::Position::maxRows-1-row)*keySize-2, colX(row+1, col+1)-colX(row-1, col)-4, 4, 0xffffff);\
				if(join&&i==0)buffer.draw_line(colX(row+1, col)+1, (keyboard::Position::maxRows-1-row)*keySize-2, colX(row+1, col)+1, (keyboard::Position::maxRows-1-row-1)*keySize+2, 0xbbbbbb);\
				if(join&&i==0)buffer.draw_line(colX(row+1, col+1)-2, (keyboard::Position::maxRows-1-row)*keySize-2, colX(row+1, col+1)-2, (keyboard::Position::maxRows-1-row-1)*keySize+2, 0xbbbbbb);\
				col++;
			#define EXTEND_DOWN \
				if(i==0)buffer.draw_rect_outline(colX(row, col)+1, (keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==1)buffer.draw_rect(colX(row, col)+2, (keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col)-4, keySize-4, 0xffffff);\
				if(join&&i==1)buffer.draw_rect(colX(row-1, col  )+2, (keyboard::Position::maxRows-1-row+1)*keySize-2, colX(row-1, col+1)-colX(row-1, col)-4, 4, 0xffffff);\
				if(join&&i==0)buffer.draw_line(colX(row-1, col  )+1, (keyboard::Position::maxRows-1-row+1)*keySize-2, colX(row-1, col  )+1, (keyboard::Position::maxRows-1-row+1)*keySize+2, 0xbbbbbb);\
				if(join&&i==0)buffer.draw_line(colX(row-1, col+1)-2, (keyboard::Position::maxRows-1-row+1)*keySize-2, colX(row-1, col+1)-2, (keyboard::Position::maxRows-1-row+1)*keySize+2, 0xbbbbbb);\
				col++;
			#define EXTEND_LEFT \
				if(!join&&i==0)buffer.draw_rect_outline(colX(row, col)+1, (keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==0)buffer.draw_rect_outline(colX(row, col)-3, (keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)+2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==1)buffer.draw_rect(colX(row, col)-3, (keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col), keySize-2-2, 0xffffff);\
				col++;
			#define EXTEND_RIGHT \
				if(!join&&i==0)buffer.draw_rect_outline(colX(row, col)+1, (keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==0)buffer.draw_rect_outline(colX(row, col)+1, (keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)+2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==1)buffer.draw_rect(colX(row, col)+3, (keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col), keySize-2-2, 0xffffff);\
				col++;

			I32 row;
			I32 col;

			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wunused-value"

			for(auto i=0;i<2;i++){
				row = 7;
				col = 0;
				KEYBOARD_LAYOUT_UK_ROW7
				while(col<keyboard::Position::maxCols) { EMPTY }
				row = 6;
				col = 0;
				KEYBOARD_LAYOUT_UK_ROW6
				while(col<keyboard::Position::maxCols) { EMPTY }
				row = 5;
				col = 0;
				KEYBOARD_LAYOUT_UK_ROW5
				while(col<keyboard::Position::maxCols) { EMPTY }
				row = 4;
				col = 0;
				KEYBOARD_LAYOUT_UK_ROW4
				while(col<keyboard::Position::maxCols) { EMPTY }
				row = 3;
				col = 0;
				KEYBOARD_LAYOUT_UK_ROW3
				while(col<keyboard::Position::maxCols) { EMPTY }
				row = 2;
				col = 0;
				KEYBOARD_LAYOUT_UK_ROW2
				while(col<keyboard::Position::maxCols) { EMPTY }
				row = 1;
				col = 0;
				KEYBOARD_LAYOUT_UK_ROW1
				while(col<keyboard::Position::maxCols) { EMPTY }
				row = 0;
				col = 0;
				KEYBOARD_LAYOUT_UK_ROW0
				while(col<keyboard::Position::maxCols) { EMPTY }
			}

			driver::Keyboard::allEvents.subscribe([](const driver::Keyboard::Event &event, void *_window){
				auto &window = *(driver::system::DesktopManager::Window*)_window;

				static char statusBuffer[256] = "";
				static char typeBuffer[32] = "Typed: ";

				switch(event.type){
					case driver::Keyboard::Event::Type::pressed: {
						strcpy(statusBuffer, "Pressed: ");
						if(event.pressed.modifiers&(U8)driver::Keyboard::Modifier::control){
							strcat(statusBuffer, "Ctrl+");
						}
						if(event.pressed.modifiers&(U8)driver::Keyboard::Modifier::alt){
							strcat(statusBuffer, "Alt+");
						}
						if(event.pressed.modifiers&(U8)driver::Keyboard::Modifier::super){
							strcat(statusBuffer, "Super+");
						}
						if(event.pressed.modifiers&(U8)driver::Keyboard::Modifier::shift){
							strcat(statusBuffer, "Shift+");
						}
						strcat(statusBuffer, get_scankey_name(event.pressed.scancode));
						window.set_status(statusBuffer);

						// logging::print_debug("pressed: ", get_scankey_name(event.pressed.scancode));
						auto position = keyboard::scancodeToPosition(event.pressed.scancode);
						auto rect = graphics2d::Rect{
							(I32)colX(position.row, position.col)+2, (keyboard::Position::maxRows-1-position.row)*keySize+2,
							(I32)colX(position.row, position.col+1)-2
						};
						rect.y2 = rect.y1 + (keySize-4);
						rect = rect.offset(padding, padding);
						rect = rect.cropped(rect.width()*5/12, rect.height()*5/12, rect.width()*5/12, rect.height()*5/12);

						window.clientArea.draw_rect(rect, 0xffc0c0);
						window.redraw_area(rect);
					} break;
					case driver::Keyboard::Event::Type::released: {
						// logging::print_debug("released: ", get_scankey_name(event.released.scancode));
						auto position = keyboard::scancodeToPosition(event.released.scancode);
						auto rect = graphics2d::Rect{
							(I32)colX(position.row, position.col)+2, (keyboard::Position::maxRows-1-position.row)*keySize+2,
						};
						rect.x2 = rect.x1 + (colX(position.row, position.col+1)-colX(position.row, position.col)-4);
						rect.y2 = rect.y1 + (keySize-4);
						rect = rect.offset(padding, padding);
						rect = rect.cropped(rect.width()*5/12, rect.height()*5/12, rect.width()*5/12, rect.height()*5/12);

						window.clientArea.draw_rect(rect, 0xffffff);
						window.redraw_area(rect);
					} break;
					case driver::Keyboard::Event::Type::actionPressed:
						strcpy(statusBuffer, "Pressed: ");
						strcat(statusBuffer, event.actionPressed.action);
						window.set_status(statusBuffer);

						// logging::print_debug("action: ", event.actionPressed.action);
					break;
					case driver::Keyboard::Event::Type::characterTyped: {
						C8 character[2] = {event.characterTyped.character<256?(C8)event.characterTyped.character:'?', '\0'};

						if(character[0]=='\n'){
							strcpy(typeBuffer, "Typed: ");

						}else if(character[0]=='\b'){
							if(strlen(typeBuffer)>7){
								typeBuffer[strlen(typeBuffer)-1]='\0';
							}

						}else{
							if(strlen(typeBuffer)+1>=sizeof(typeBuffer)){
								memcpy(typeBuffer+7, typeBuffer+sizeof(typeBuffer)/2, sizeof(typeBuffer)/2);
							}
							strcat(typeBuffer, character);
						}

						window.set_status(typeBuffer);
					} break;
				}

			}, window);

			#pragma GCC diagnostic pop

			window->redraw();
		}

		#undef KEYBOARD_LAYOUT_ROW0
		#undef KEYBOARD_LAYOUT_ROW1
		#undef KEYBOARD_LAYOUT_ROW2
		#undef KEYBOARD_LAYOUT_ROW3
		#undef KEYBOARD_LAYOUT_ROW4
		#undef KEYBOARD_LAYOUT_ROW5
		#undef KEYBOARD_LAYOUT_ROW6
		#undef KEYBOARD_LAYOUT_ROW7
	}
}
