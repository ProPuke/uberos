#include "keyboardTest.hpp"

#include <drivers/DesktopManager.hpp>
#include <drivers/Keyboard.hpp>

#include <kernel/drivers.hpp>
#include <kernel/keyboard.hpp>
#include <kernel/keyboard/layout/uk.hpp>

#include <common/graphics2d/font.hpp>

namespace tests::keyboardTest {
	namespace {
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

		driver::DesktopManager *desktopManager;
	}

	void run() {
		desktopManager = drivers::find_and_activate<driver::DesktopManager>();
		if(!desktopManager) return;

		const static auto padding = 6;

		static auto window = &desktopManager->create_standard_window("Keyboard Test", 888+padding*2, 292+padding*2);

		static auto keySize = 30;
		static auto offsetX = 0;
		static auto offsetY = 0;

		static bool staggered = true;
		static bool join = true;

		static const auto colX = [=](U32 row, U32 col) -> U32 {
			if(!staggered) return offsetX + col*keySize;

			if(col>5&&col<20&&row==0){
				return offsetX + col*keySize+(+keySize*1/3*(14-(col-5))/14)-(keySize*4/8*(col-5)/14);

			}else if(col>5&&col<20&&row==1){
				return offsetX + col*keySize+keySize*2/8;

			}else if(col>5&&col<20&&row==2){
				return offsetX + col*keySize-(col>6?keySize*2/8:keySize*1/8);

			}else if(col>5&&col<20&&row==3){
				return offsetX + col*keySize-(col>6?keySize*1/2:keySize*1/4);
				// return offsetX + col*keySize-keySize*5/8;

			}else{
				return offsetX + col*keySize;
			}
		};

		static auto redraw = [](){
			U32 corner[] = {1, (U32)-1};
			auto &clientArea = window->get_client_area();

			keySize = min(
				(clientArea.width-padding*2)/29,
				(clientArea.height-padding*2)/8
			);
			const auto requiredWidth = 29 * keySize;
			const auto requiredHeight = 8 * keySize;

			// auto buffer = clientArea.cropped(padding, padding, padding, padding);
			offsetX = (clientArea.width-requiredWidth)/2;
			offsetY = (clientArea.height-requiredHeight)/2;

			#define KEY(NAME,...) \
				if(i==0)clientArea.draw_rect_outline(colX(row, col)+1, offsetY+(keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(i==1)clientArea.draw_rect(colX(row, col)+2, offsetY+(keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col)-4, keySize-4, 0xffffff);\
				if(i==1&&(false,##__VA_ARGS__))clientArea.draw_text({.font=*graphics2d::font::default_sans, .size=8}, ("",##__VA_ARGS__), colX(row, col)+1+3, offsetY+(keyboard::Position::maxRows-1-row)*keySize+3+8, keySize-6, 0x222222);\
				col++;
			#define EMPTY \
				if(i==0)clientArea.draw_rect_outline(colX(row, col)+1, offsetY+(keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xdddddd, 1, corner, corner, corner, corner);\
				col++;
			#define EXTEND_UP \
				if(i==0)clientArea.draw_rect_outline(colX(row, col)+1, offsetY+(keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==1)clientArea.draw_rect(colX(row, col)+2, offsetY+(keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col)-4, keySize-4, 0xffffff);\
				if(join&&i==1)clientArea.draw_rect(colX(row+1, col)+2, offsetY+(keyboard::Position::maxRows-1-row)*keySize-2, colX(row+1, col+1)-colX(row-1, col)-4, 4, 0xffffff);\
				if(join&&i==0)clientArea.draw_line(colX(row+1, col)+1, offsetY+(keyboard::Position::maxRows-1-row)*keySize-2, colX(row+1, col)+1, (keyboard::Position::maxRows-1-row-1)*keySize+2, 0xbbbbbb);\
				if(join&&i==0)clientArea.draw_line(colX(row+1, col+1)-2, offsetY+(keyboard::Position::maxRows-1-row)*keySize-2, colX(row+1, col+1)-2, (keyboard::Position::maxRows-1-row-1)*keySize+2, 0xbbbbbb);\
				col++;
			#define EXTEND_DOWN \
				if(i==0)clientArea.draw_rect_outline(colX(row, col)+1, offsetY+(keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==1)clientArea.draw_rect(colX(row, col)+2, offsetY+(keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col)-4, keySize-4, 0xffffff);\
				if(join&&i==1)clientArea.draw_rect(colX(row-1, col  )+2, offsetY+(keyboard::Position::maxRows-1-row+1)*keySize-2, colX(row-1, col+1)-colX(row-1, col)-4, 4, 0xffffff);\
				if(join&&i==0)clientArea.draw_line(colX(row-1, col  )+1, offsetY+(keyboard::Position::maxRows-1-row+1)*keySize-2, colX(row-1, col  )+1, (keyboard::Position::maxRows-1-row+1)*keySize+2, 0xbbbbbb);\
				if(join&&i==0)clientArea.draw_line(colX(row-1, col+1)-2, offsetY+(keyboard::Position::maxRows-1-row+1)*keySize-2, colX(row-1, col+1)-2, (keyboard::Position::maxRows-1-row+1)*keySize+2, 0xbbbbbb);\
				col++;
			#define EXTEND_LEFT \
				if(!join&&i==0)clientArea.draw_rect_outline(colX(row, col)+1, offsetY+(keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==0)clientArea.draw_rect_outline(colX(row, col)-3, offsetY+(keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)+2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==1)clientArea.draw_rect(colX(row, col)-3, offsetY+(keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col), keySize-2-2, 0xffffff);\
				col++;
			#define EXTEND_RIGHT \
				if(!join&&i==0)clientArea.draw_rect_outline(colX(row, col)+1, offsetY+(keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)-2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==0)clientArea.draw_rect_outline(colX(row, col)+1, offsetY+(keyboard::Position::maxRows-1-row)*keySize+1, colX(row, col+1)-colX(row, col)+2, keySize-2, 0xbbbbbb, 1, corner, corner, corner, corner);\
				if(join&&i==1)clientArea.draw_rect(colX(row, col)+3, offsetY+(keyboard::Position::maxRows-1-row)*keySize+2, colX(row, col+1)-colX(row, col), keySize-2-2, 0xffffff);\
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
		};

		redraw();

		window->events.subscribe([](const driver::DesktopManager::Window::Event &event){
			if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
				redraw();
			}
		});

		driver::Keyboard::allEvents.subscribe([](const driver::Keyboard::Event &event, void *_window){
			auto &window = *(driver::DesktopManager::StandardWindow*)_window;
			auto &clientArea = window.get_client_area();

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
						(I32)colX(position.row, position.col)+2, offsetY+(keyboard::Position::maxRows-1-position.row)*keySize+2,
						(I32)colX(position.row, position.col+1)-2
					};
					rect.y2 = rect.y1 + (keySize-4);
					rect = rect.cropped(rect.width()*5/12, rect.height()*5/12, rect.width()*5/12, rect.height()*5/12);

					clientArea.draw_rect(rect, 0xffc0c0);
					window.redraw_area(rect);
				} break;
				case driver::Keyboard::Event::Type::released: {
					// logging::print_debug("released: ", get_scankey_name(event.released.scancode));
					auto position = keyboard::scancodeToPosition(event.released.scancode);
					auto rect = graphics2d::Rect{
						(I32)colX(position.row, position.col)+2, offsetY+(keyboard::Position::maxRows-1-position.row)*keySize+2,
					};
					rect.x2 = rect.x1 + (colX(position.row, position.col+1)-colX(position.row, position.col)-4);
					rect.y2 = rect.y1 + (keySize-4);
					rect = rect.cropped(rect.width()*5/12, rect.height()*5/12, rect.width()*5/12, rect.height()*5/12);

					clientArea.draw_rect(rect, 0xffffff);
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
}
