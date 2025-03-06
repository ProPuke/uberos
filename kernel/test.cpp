#include <drivers/Clock.hpp>
#include <drivers/DesktopManager.hpp>
#include <drivers/Keyboard.hpp>
#include <drivers/Mouse.hpp>

#include <kernel/drivers.hpp>
#include <kernel/keyboard.hpp>
#include <kernel/keyboard/layout/uk.hpp>

#include <common/graphics2d.hpp>
#include <common/graphics2d/font.hpp>

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
		if(auto desktopManager = drivers::find_and_activate<driver::DesktopManager>()) {
			const auto width = 600;
			const auto height = 150;
			const static auto shadowLength = 8;
			const static auto transparentBackgroundColour = graphics2d::premultiply_colour(desktopManager->get_default_window_colour()|(0xff-0x44)<<24);
			const static auto opaqueBackgroundColour = graphics2d::premultiply_colour(desktopManager->get_default_window_colour()|0x44<<24);
			// const static auto borderColour = desktopManager->get_default_window_border_colour();
			const static auto borderColour = graphics2d::premultiply_colour(0xeeeeee|(255-0x88)<<24);
			const static auto opaqueBorderColour = graphics2d::premultiply_colour(0xeeeeee|(255-0xdd)<<24);

			static auto window = &desktopManager->create_custom_window("Taskbar", width, height+shadowLength);
			window->set_margin(shadowLength, shadowLength, shadowLength, shadowLength);
			window->set_interact_area({shadowLength,shadowLength,shadowLength+width,shadowLength+height});
			window->set_titlebar_area({shadowLength,shadowLength,shadowLength+width,shadowLength+height});
			window->set_max_docked_size(170, 70);
			window->dock(driver::DesktopManager::Window::DockedType::top);
			window->set_layer(driver::DesktopManager::Window::Layer::topmost);

			static auto redraw = [](){
				auto width = window->get_width();
				auto height = window->get_height();
				auto &clientArea = window->get_client_area();
				auto &windowArea = window->get_window_area();

				if(window->get_state()==driver::DesktopManager::Window::State::docked){
					window->set_solid_area({0,0,0,0});
					window->set_titlebar_area({0, 0, width, height});

					clientArea.draw_rect(0, 0, width, height, transparentBackgroundColour);

					switch(window->get_docked_type()){
						case driver::DesktopManager::Window::DockedType::top:
							windowArea.set(shadowLength, shadowLength+height-1, borderColour, width);
							for(auto i=0;i<shadowLength;i++){
								windowArea.set(shadowLength, shadowLength+height+i, 0x000000|(255-20+20*i/(shadowLength-1))<<24, width);
							}
						break;
						case driver::DesktopManager::Window::DockedType::bottom:
							windowArea.set(shadowLength, shadowLength, borderColour, width);
							for(auto i=0;i<shadowLength;i++){
								windowArea.set(0, i, 0x000000|(255-20*i/(shadowLength-1))<<24, width);
							}
						break;
						case driver::DesktopManager::Window::DockedType::left:
							windowArea.draw_line(shadowLength+width-1, shadowLength, shadowLength+width-1, shadowLength+height-1, borderColour);
							for(auto i=0;i<shadowLength;i++){
								windowArea.draw_line(
									shadowLength+width+i, shadowLength+0,
									shadowLength+width+i, shadowLength+height-1,
									0x000000|(255-20+20*i/(shadowLength-1))<<24
								);
							}
						break;
						case driver::DesktopManager::Window::DockedType::right:
							windowArea.draw_line(shadowLength, shadowLength, shadowLength, shadowLength+height-1, borderColour);
							for(auto i=0;i<shadowLength;i++){
								windowArea.draw_line(
									i, shadowLength+0,
									i, shadowLength+height-1,
									0x000000|(255-20*i/(shadowLength-1))<<24
								);
							}
						break;
						case driver::DesktopManager::Window::DockedType::full:
						break;
					}

				}else{
					// window->set_solid_area({shadowLength, shadowLength, shadowLength+width, shadowLength+height});
					// clientArea.draw_rect(1, 1, width-1, height-1, solidBackgroundColour);

					window->set_solid_area({0,0,0,0});
					window->set_titlebar_area({0, 0, width, height});
					clientArea.draw_rect(0, 0, width, height, opaqueBackgroundColour);

					clientArea.draw_rect_outline(0, 0, width, height, opaqueBorderColour);

					for(auto i=0;i<shadowLength;i++){
						windowArea.set(shadowLength, shadowLength+height+i, 0x000000|(255-20+20*i/(shadowLength-1))<<24, width);
					}
				}

				{
					char time[] = "00:00";

					auto clock = drivers::find_and_activate<driver::Clock>();
					if(clock){
						auto clockTime = clock->get_time();
						auto clockDate = clock->get_date();
						(void)clockDate;
						
						auto str = utoa((U16)clockTime.hours);
						if(str[1]=='\0'){
							time[1] = str[0];
						}else{
							time[0] = str[0];
							time[1] = str[1];
						}

						str = utoa((U16)clockTime.minutes);
						if(str[1]=='\0'){
							time[4] = str[0];
						}else{
							time[3] = str[0];
							time[4] = str[1];
						}
					}

					auto fontSettings = graphics2d::Buffer::FontSettings{
						.font = graphics2d::font::manrope_extraBold,
						.size = 48
					};

					auto fontMeasurements = clientArea.measure_text(fontSettings, time, 0, 0, clientArea.width);

					if(clientArea.width>=clientArea.height){
						clientArea.draw_text(fontSettings, time, width-fontMeasurements.maxX-12+1, height-fontMeasurements.updatedArea.y2-(height-fontMeasurements.updatedArea.y2+fontMeasurements.updatedArea.y1)/2, fontMeasurements.maxX, 0x000000);
						clientArea.draw_text(fontSettings, time, width-fontMeasurements.maxX-12, height-fontMeasurements.updatedArea.y2-(height-fontMeasurements.updatedArea.y2+fontMeasurements.updatedArea.y1)/2-1, fontMeasurements.maxX, 0xffffff);
					}else{
						clientArea.draw_text(fontSettings, time, (width-fontMeasurements.maxX)/2+1, height-fontMeasurements.updatedArea.y2-fontMeasurements.updatedArea.y2-12+1, fontMeasurements.maxX, 0x000000);
						clientArea.draw_text(fontSettings, time, (width-fontMeasurements.maxX)/2, height-fontMeasurements.updatedArea.y2-fontMeasurements.updatedArea.y2-12, fontMeasurements.maxX, 0xffffff);
					}
				}
			};

			redraw();
			window->redraw();

			window->events.subscribe([](const driver::DesktopManager::Window::Event &event){
				if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
					redraw();
				}
			});
		}

		if(auto desktopManager = drivers::find_and_activate<driver::DesktopManager>()) {
			static auto window = &desktopManager->create_standard_window("Font Test", 320, 320);
			// view = graphics2d::create_view(nullptr, graphics2d::DisplayLayer::topMost, margin, margin, min(1300u, framebuffer.buffer.width-margin*2), 256);

			static auto scale = 30u;

			static auto redraw = [](){
				auto &clientArea = window->get_client_area();

				clientArea.draw_rect(0, 0, window->get_width(), window->get_height(), window->get_background_colour());

				auto fontSettings = graphics2d::Buffer::FontSettings{
					.font = *graphics2d::font::default_sans,
					.size = scale
				};

				for(auto y=scale;y<(U32)window->get_height();y++){
					if(fontSettings.size>24){
						clientArea.draw_text(fontSettings, "Abc", 10+1, y+1, 320, 0x222222);
						clientArea.draw_text(fontSettings, "Abc", 10, y, 320, 0xaaaaaa);
					}else{
						clientArea.draw_text(fontSettings, "Abc", 10, y, 320, 0x222222);
					}

					fontSettings.size += scale;
					y += fontSettings.size * 4 / 5;
				}

				static char statusBuffer[64];
				strcpy(statusBuffer, "Step: ");
				strcat(statusBuffer, utoa(scale));
				window->set_status(statusBuffer);

				window->redraw();
			};

			redraw();

			window->events.subscribe([](const driver::DesktopManager::Window::Event &event, void*){
				if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
					redraw();

				}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseScrolled){
					scale = maths::clamp(scale - event.mouseScrolled.distance*(scale>20?3:scale>16?2:1), 1, 60);
					redraw();
				}

			}, nullptr);
		}

		if(auto desktopManager = drivers::find_and_activate<driver::DesktopManager>()) {
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
