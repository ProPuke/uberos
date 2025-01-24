#include "DesktopManager.hpp"

#include <kernel/drivers/Mouse.hpp>
#include <kernel/drivers.hpp>

#include <common/graphics2d/font.hpp>
#include <common/maths.hpp>

namespace ui2d::image {
	namespace cursors {
		extern graphics2d::Buffer _default;
		extern graphics2d::Buffer _default_left;
		extern graphics2d::Buffer _default_right;
	}
}

namespace driver::system {
	namespace {
		DriverReference<DisplayManager> displayManager{nullptr, [](void*){

		}, nullptr};

		struct Cursor;

		struct Window: LListItem<Window>, DesktopManager::Window {
			DisplayManager::Display *graphicsDisplay;

			const char *title;
			Cursor *draggingCursor = nullptr;

			/**/ Window(U32 x, U32 y, U32 width, U32 height, const char *title):
				graphicsDisplay(displayManager->create_display(nullptr, DisplayManager::DisplayLayer::regular, x, y, width+leftShadow+rightShadow, height+bottomShadow)),
				title(title)
			{
				if(graphicsDisplay){
					clientArea = graphicsDisplay->buffer;
				}

				// graphics2d::Buffer::create_round_corner(cornerRadius, corner);
				graphics2d::Buffer::create_diagonal_corner(cornerRadius, corner);
				graphics2d::Buffer::create_diagonal_corner(cornerRadius-1, cornerInner);

				memcpy(graphicsDisplay->topLeftCorner, corner, sizeof(corner)-sizeof(corner[0]));
				memcpy(graphicsDisplay->topRightCorner, corner, sizeof(corner)-sizeof(corner[0]));
				memcpy(graphicsDisplay->bottomLeftCorner, corner, sizeof(corner)-sizeof(corner[0]));
				memcpy(graphicsDisplay->bottomRightCorner, corner, sizeof(corner)-sizeof(corner[0]));
			}

			static const auto cornerRadius = 2;
			U32 corner[cornerRadius+1];
			U32 cornerInner[cornerRadius-1+1];

			U32 leftShadow = 8;
			U32 rightShadow = 8;
			U32 bottomShadow = 12;

			U8 shadowIntensity = 72; // max intensity
			U8 leftShadowIntensity = 50; // scaling down of left shadow (by inner extension)
			U8 rightShadowIntensity = 50; // scaling down of right shadow (by inner extension)

			auto get_x() -> I32 override {
				return graphicsDisplay->x+leftShadow;
			}

			auto get_y() -> I32 override {
				return graphicsDisplay->y;
			}

			auto get_width() -> I32 override {
				return graphicsDisplay->get_width()-leftShadow-rightShadow;
			}

			auto get_height() -> I32 override {
				return graphicsDisplay->get_height()-bottomShadow;
			}

			auto get_border_rect() -> graphics2d::Rect {
				return graphics2d::Rect{(I32)leftShadow, 0, (I32)leftShadow+get_width(), get_height()};
			}

			void set_title(const char*) override;

			void show() override;
			void hide() override;
			void move_to(I32 x, I32 y) override;

			void redraw() override;
			void redraw_area(graphics2d::Rect) override;

			void _draw_border() {
				auto rect = get_border_rect();
				const auto borderColour = draggingCursor?0xd0b0b0:0xcbcbcb;

				graphicsDisplay->buffer.draw_rect_outline(rect.x1, rect.y1, get_width(), get_height(), borderColour, 1, corner, corner, corner, corner);
			}

			void _draw_titlebar() {
				auto rect = get_border_rect();
				const auto borderColour = draggingCursor?0xd0b0b0:0xcbcbcb;
				const auto bgColour = draggingCursor?0xfff9f9:0xf9f9f9;

				graphicsDisplay->buffer.draw_rect_outline(rect.x1, rect.y1, get_width(), titlebarHeight, borderColour, 1, corner, corner, nullptr, nullptr);
				graphicsDisplay->buffer.draw_rect(rect.x1+1, rect.y1+1, get_width()-2, titlebarHeight-2, bgColour, cornerInner, cornerInner, nullptr, nullptr);

				auto lineHeight = 14*5/4;
				const auto width = graphicsDisplay->buffer.measure_text(*graphics2d::font::default_sans, title, 0, 0, 14).x;
				graphicsDisplay->buffer.draw_text(*graphics2d::font::default_sans, title, rect.x1+get_width()/2-width/2, rect.y1+1+lineHeight, get_width(), 14, 0x333333);
			}

			void _draw_statusbar() {
				auto rect = get_border_rect();
				const auto borderColour = 0xcbcbcb;

				graphicsDisplay->buffer.draw_rect_outline(rect.x1+0, rect.y1+get_height()-1-21-1, get_width(), 23, borderColour, 1, nullptr, nullptr, corner, corner);
				graphicsDisplay->buffer.draw_rect(rect.x1+1, rect.y1+get_height()-1-21, get_width()-2, 21, 0xeeeeee, nullptr, nullptr, cornerInner, cornerInner);
			}

			void redraw_border() {
				auto rect = get_border_rect();

				_draw_border();
				//top
				graphicsDisplay->update_area(graphics2d::Rect{0, 0, (I32)get_width(), cornerRadius}.offset(rect.x1, rect.y1));
				//left
				graphicsDisplay->update_area(graphics2d::Rect{0, cornerRadius, 1, (I32)get_height()-cornerRadius}.offset(rect.x1, rect.y1));
				//right
				graphicsDisplay->update_area(graphics2d::Rect{(I32)get_width()-1, cornerRadius, (I32)get_width(), (I32)get_height()-cornerRadius}.offset(rect.x1, rect.y1));
				//bottom
				graphicsDisplay->update_area(graphics2d::Rect{0, (I32)get_height()-1, (I32)get_width(), (I32)get_height()}.offset(rect.x1, rect.y1));
			}

			void redraw_titlebar() {
				_draw_titlebar();
				auto rect = get_border_rect();

				graphicsDisplay->update_area(graphics2d::Rect{0, 0, (I32)get_width(), titlebarHeight}.offset(rect.x1, rect.y1));
			}

			static const auto titlebarHeight = 28;

			void draw_frame() {
				// auto rect = get_border_rect();

				// U32 corner[4] = {3, 1, 1, (U32)~0};
				// U32 cornerInner[3] = {2, 1, (U32)~0};

				auto &display = *graphicsDisplay;

				_draw_border();
				_draw_titlebar();
				_draw_statusbar();
				_draw_shadow();

				clientArea = display.buffer.cropped(leftShadow+1, titlebarHeight, rightShadow+1, bottomShadow+23);
				clientArea.draw_rect(0, 0, clientArea.width, clientArea.height, 0xeeeeee);
				display.update();
			}

			auto get_shadow_intensity_at(I32 x, I32 y) -> U8 {
				U8 intensity = 255;

				//we lengthen it inward when the intensity is turned down (so it fades in nicely at the bottom)
				auto leftShadowLength = (I32)leftShadow * 255/(I32)leftShadowIntensity;
				auto rightShadowLength = (I32)rightShadow * 255/(I32)rightShadowIntensity;

				if(x<(I32)leftShadowLength&&x<(I32)graphicsDisplay->get_width()/2){
					intensity = intensity * (x+1)/leftShadowLength;

				}else if(x>=(I32)graphicsDisplay->get_width()-rightShadowLength&&x>=(I32)graphicsDisplay->get_width()/2){
					intensity = intensity * (leftShadow+get_width()+rightShadow-x)/rightShadowLength;
				}

				const auto topShadow = bottomShadow*2; // top fade twice of bottom

				if(y<(I32)topShadow){
					// use for top fade, as well
					intensity = intensity * (y+1)/topShadow;

				}else if(y>=get_height()){
					intensity = intensity * (get_height()+bottomShadow-y)/bottomShadow;
				}

				return intensity;
			}

			void _draw_shadow() {
				for(auto i=0u;i<sizeof(graphicsDisplay->topLeftCorner)/sizeof(graphicsDisplay->topLeftCorner[0]);i++){
					graphicsDisplay->topLeftCorner[i] = 0;
					graphicsDisplay->topRightCorner[i] = 0;
					graphicsDisplay->bottomLeftCorner[i] = 0;
					graphicsDisplay->bottomRightCorner[i] = 0;
				}
				graphicsDisplay->solidArea = {(I32)leftShadow+cornerRadius, cornerRadius, (I32)leftShadow+get_width()-cornerRadius, get_height()-(I32)bottomShadow-cornerRadius};

				auto &buffer = graphicsDisplay->buffer;
				for(auto y=0; y<(I32)buffer.height; y++){
					const auto cornerIndent = y<cornerRadius?(I32)corner[y]:y<get_height()&&(I32)get_height()-1-y<cornerRadius?(I32)corner[(I32)get_height()-1-y]:0;

					for(auto x=0; x<(I32)leftShadow+cornerIndent; x++){
						buffer.set(x, y, 0x000000|(255-(get_shadow_intensity_at(x, y)*shadowIntensity/255)<<24));
					}
					for(auto x=0; x<(I32)rightShadow+cornerIndent; x++){
						buffer.set((I32)leftShadow+(I32)get_width()+(I32)rightShadow-1-x, y, 0x000000|(255-(get_shadow_intensity_at(x, y)*shadowIntensity/255)<<24));
					}
				}

				for(auto y=0; y<(I32)bottomShadow; y++){
					// buffer.set((I32)leftShadow, get_height()+y, 0x000000|(255-(get_shadow_intensity_at(leftShadow, get_height()+y)*shadowIntensity/255)<<24), get_width());

					for(auto x=0;x<get_width();x++){
						buffer.set((I32)leftShadow+x, get_height()+y, 0x000000|(255-(get_shadow_intensity_at(leftShadow+x, get_height()+y)*shadowIntensity/255)<<24));
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

			void redraw_shadow() {
				auto &buffer = graphicsDisplay->buffer;

				graphicsDisplay->update_area({0, 0, (I32)leftShadow, (I32)buffer.height});
				graphicsDisplay->update_area({(I32)leftShadow+get_width(), 0, (I32)leftShadow+get_width()+(I32)rightShadow, (I32)buffer.height});
				graphicsDisplay->update_area({(I32)leftShadow, get_height(), get_width(), get_height()+(I32)bottomShadow});
			}
		};

		struct Cursor {
			Mouse *mouse = nullptr;
			DisplayManager::Display *display;
			I32 x, y;
			bool isVisible = false;

			struct {
				Window *window;
				I32 dragOffsetX;
				I32 dragOffsetY;
			} dragWindow;

			void grab_window(Window &window) {
				if(window.draggingCursor&&window.draggingCursor!=this){
					window.draggingCursor->release_window();
				}

				dragWindow.window = &window;
				dragWindow.dragOffsetX = window.get_x()-x;
				dragWindow.dragOffsetY = window.get_y()-y;

				window.draggingCursor = this;
				window.redraw_border();
				window.redraw_titlebar();
			}

			void release_window() {
				if(!dragWindow.window||dragWindow.window->draggingCursor!=this) return;

				dragWindow.window->draggingCursor = nullptr;
				dragWindow.window->redraw_border();
				dragWindow.window->redraw_titlebar();
				dragWindow.window = nullptr;
			}
		};

		LList<Window> windows;
		ListUnordered<Cursor> cursors(0); // do not preallocate any (dynamic allocations on boot fail before memory is setup)

		void _add_mouse(Mouse &mouse);
		void _remove_mouse(Mouse &mouse);
		auto _find_cursor(Mouse &mouse) -> Cursor*;

		void _on_drivers_event(const drivers::Event &event, void*) {
			switch(event.type){
				case drivers::Event::Type::driver_started:
					if(event.driver_started.driver->is_type(Mouse::typeInstance)){
						_add_mouse(*(Mouse*)event.driver_started.driver);
					}
				break;
				case drivers::Event::Type::driver_stopped:
					if(event.driver_started.driver->is_type(Mouse::typeInstance)){
						_remove_mouse(*(Mouse*)event.driver_started.driver);
					}
				break;
			}
		}

		void _on_cursor_mouse_event(const Mouse::Event &event, void *_mouse) {
			auto cursor = _find_cursor(*(Mouse*)_mouse);
			if(!cursor) return;

			switch(event.type){
				case Mouse::Event::Type::pressed:
					if(event.pressed.button==0){
						{
							auto &cursorImage = ui2d::image::cursors::_default_left;
							cursor->display->buffer.draw_buffer_area(0, 0, 0, 0, cursorImage.width, cursorImage.height, cursorImage);
							cursor->display->update();
						}
						if(!cursor->dragWindow.window){
							if(auto display = displayManager->get_display_at(cursor->x, cursor->y, cursor->display, false)){
								if(auto window = (Window*)DesktopManager::instance.get_window_from_display(*display)){
									cursor->grab_window(*window);
								}
							}
						}

					}else if(event.pressed.button==1){
						{
							auto &cursorImage = ui2d::image::cursors::_default_right;
							cursor->display->buffer.draw_buffer_area(0, 0, 0, 0, cursorImage.width, cursorImage.height, cursorImage);
							cursor->display->update();
						}
					}
				break;
				case Mouse::Event::Type::released:
					if(event.released.button==0||event.released.button==1){
						auto &cursorImage = ui2d::image::cursors::_default;
						cursor->display->buffer.draw_buffer_area(0, 0, 0, 0, cursorImage.width, cursorImage.height, cursorImage);
							cursor->display->update();
					}

					if(event.released.button==0){
						if(cursor->dragWindow.window){
							cursor->release_window();
						}
					}
				case Mouse::Event::Type::scrolled:
				break;
				case Mouse::Event::Type::moved:
					cursor->x = maths::clamp(cursor->x+event.moved.x, 0, (I32)displayManager->get_width()-1);
					cursor->y = maths::clamp(cursor->y+event.moved.y, 0, (I32)displayManager->get_height()-1);
					cursor->display->move_to(cursor->x, cursor->y);

					if(cursor->dragWindow.window){
						cursor->dragWindow.window->move_to(cursor->x+cursor->dragWindow.dragOffsetX, cursor->y+cursor->dragWindow.dragOffsetY);
					}

					if(!cursor->isVisible){
						cursor->isVisible = true;
						cursor->display->show();
					}
				break;
			}
		}

		void _add_mouse(Mouse &mouse) {
			if(!displayManager) return;

			auto &log = DesktopManager::instance.log;

			log.print_info("mouse added");

			auto x = 0;//(I32)displayManager->get_width()/2;
			auto y = (I32)displayManager->get_height()/2;
			auto &cursorImage = ui2d::image::cursors::_default;

			auto cursorDisplay = displayManager->create_display(nullptr, DisplayManager::DisplayLayer::cursor, x, y, cursorImage.width, cursorImage.height);
			// cursorDisplay->mode = DisplayManager::DisplayMode::transparent; // TODO: add some kind of api for specifically marking displays as transparent
			cursorDisplay->solidArea.clear();
			cursorDisplay->buffer.draw_buffer_area(0, 0, 0, 0, cursorImage.width, cursorImage.height, cursorImage);
			cursorDisplay->hide();

			cursors.push({&mouse, cursorDisplay, x, y, false});

			mouse.events.subscribe(_on_cursor_mouse_event, &mouse);
		}

		void _remove_mouse(Mouse &mouse) {
			auto &log = DesktopManager::instance.log;

			for(auto i=0u;i<cursors.length;i++){
				auto &cursor = cursors[i];
				if(cursor.mouse==&mouse){
					log.print_info("mouse removed");
					delete cursor.display;
					cursor.mouse->events.unsubscribe(_on_cursor_mouse_event, &mouse);
					cursors.remove(i);
					return;
				}
			}
		}

		auto _find_cursor(Mouse &mouse) -> Cursor* {
			for(auto &cursor:cursors) {
				if(cursor.mouse==&mouse) return &cursor;
			}
			return nullptr;
		}

		auto client_to_graphics_area(Window &window, graphics2d::Rect rect) {
			auto &display = *window.graphicsDisplay;

			const auto addressOffset = window.clientArea.address - display.buffer.address;
			auto yOffset = addressOffset / display.buffer.stride;
			auto xOffset = (addressOffset - (yOffset * display.buffer.stride)) / graphics2d::bufferFormat::size[(U32)display.buffer.format];

			return rect.offset(xOffset, yOffset);
		}
	}

	auto DesktopManager::_on_start() -> Try<> {
		displayManager = drivers::find_and_activate<DisplayManager>();
		if(!displayManager) return {"Display manager not available"};

		for(auto &mouse:drivers::iterate<Mouse>()){
			if(!mouse.api.is_active()&&mouse.api.is_enabled()){
				if(!drivers::start_driver(mouse)) continue;
			}

			_add_mouse(mouse);
		}

		drivers::events.subscribe(_on_drivers_event, nullptr);

		return {};
	}

	auto DesktopManager::_on_stop() -> Try<> {
		// remove all cursors and mouse listeners
		while(cursors.length>0) _remove_mouse(*cursors[0].mouse);

		return {};
	}

	void Window::set_title(const char *set) {
		if(title==set) return;

		title = set;
	}

	void Window::show() {
		graphicsDisplay->show();
	}
	
	void Window::hide() {
		graphicsDisplay->hide();
	}

	void Window::move_to(I32 x, I32 y) {
		const auto margin = 10;
		x = maths::clamp(x, -(I32)graphicsDisplay->buffer.width+margin, (I32)displayManager->get_width()-margin);
		y = maths::clamp(y, -10, (I32)displayManager->get_height()-margin);

		graphicsDisplay->move_to(x-leftShadow, y);
	}
	
	void Window::redraw() {
		graphicsDisplay->update_area(client_to_graphics_area(*this, {0, 0, (I32)clientArea.width, (I32)clientArea.height}));
	}

	void Window::redraw_area(graphics2d::Rect rect) {
		graphicsDisplay->update_area(client_to_graphics_area(*this, rect));
	}

	auto DesktopManager::create_window(const char *title, I32 width, I32 height, I32 x, I32 y) -> Window& {
		// centre by default
		{
			if(x==(I32)~0){
				x = ((I32)displayManager->get_width()-width)/2;
			}

			if(y==(I32)~0){
				y = max(0, ((I32)displayManager->get_height()-height)/2);
			}
		}

		auto &window = *new system::Window(x, y, width, height, title);

		window.draw_frame();

		windows.push_back(window);
		return window;
	}

	auto DesktopManager::get_window_from_display(DisplayManager::Display &display) -> Window* {
		for(auto window=windows.head; window; window=window->next) {
			if(window->graphicsDisplay==&display) return window;
		}

		return nullptr;
	}
}
