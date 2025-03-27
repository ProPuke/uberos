#include "DesktopManager.hpp"

#include <drivers/Keyboard.hpp>
#include <drivers/Mouse.hpp>

#include <kernel/drivers.hpp>
#include <kernel/DriverReference.hpp>
#include <kernel/time.hpp>

#include <common/graphics2d.hpp>
#include <common/graphics2d/font.hpp>
#include <common/maths.hpp>
#include <common/ui2d/Gui.hpp>
#include <common/ui2d/theme/Clean.hpp>
#include <common/ui2d/control/ColouredButton.hpp>

namespace ui2d::image {
	namespace cursors {
		extern graphics2d::Buffer _default;
		extern graphics2d::Buffer _default_left;
		extern graphics2d::Buffer _default_right;
	}

	namespace widgets {
		extern graphics2d::Buffer close;
		extern graphics2d::Buffer maximise;
		extern graphics2d::Buffer minimise;
	}
}

namespace driver {
	namespace {
		DriverReference<DisplayManager> displayManager{nullptr, [](void*){

		}, nullptr};

		void update_focused_window();

		const auto enableTransparency = true;

		const U8 corner5x5Graphic[5*5] = "\000\000!\231\342\000G\353\202'!\353=\000\000\231\203\000\000\000\342(\000\000";

		struct Cursor;

		const U32 windowBackgroundColour = 0xeeeeee;
		const U32 windowBorderColour = 0xbcbcbc;
		const U32 edgeSnapDistance = 16;

		graphics2d::Rect windowArea;

		struct Window;

		extern Window *focusedWindow;

		ui2d::theme::Clean theme;

		struct DeferWindowMove {
			/**/ DeferWindowMove(Window &window);
			/**/~DeferWindowMove();

			static inline U32 depth = 0;

			Window &window;
			graphics2d::Rect oldRect;
			U8 *oldBuffer;
			U32 oldBufferWidth;
			U32 oldBufferHeight;
		};

		struct Window: LListItem<Window>, virtual DesktopManager::Window {
			U64 timeCreated = time::now();

			struct Gui: ui2d::Gui {
				typedef ui2d::Gui Super;

				/**/ Gui():
					Super(graphics2d::Buffer(), driver::theme)
				{}

				void update_area(graphics2d::Rect rect) {
					if(isFrozen) return;
					if(DeferWindowMove::depth<1){
						window().graphicsDisplay->update_area(rect);
					}
				}

				auto window() -> Window& {
					return parent(*this, &Window::gui);
				}
			} gui;

			/**/ Window(const char *title, U32 width, U32 height):
				graphicsDisplay(displayManager->create_display(nullptr, DisplayManager::DisplayLayer::regular, width, height)),
				title(title)
			{}

			auto get_x() -> I32 override {
				return graphicsDisplay->x+(I32)leftMargin;
			}

			auto get_y() -> I32 override {
				return graphicsDisplay->y+(I32)topMargin;
			}

			auto get_width() -> I32 override {
				return graphicsDisplay->get_width()-(I32)leftMargin-(I32)rightMargin;
			}

			auto get_height() -> I32 override {
				return graphicsDisplay->get_height()-(I32)topMargin-(I32)bottomMargin;
			}

			auto get_client_area() -> graphics2d::Buffer& override {
				return clientArea;
			}

			void move_to(I32 x, I32 y) override;
			void resize_to(U32 width, U32 height) override;
			void move_and_resize_to(I32 x, I32 y, U32 width, U32 height) override;

			void dock(DockedType type) override;
			void minimise() override;
			void restore() override;

			void set_title(const char *set) override {
				title = set;
				redraw_decorations(); //TODO: only update the text itself, not the full frame?
			}

			auto get_title() -> const char * override {
				return title;
			}

			// raise the current window, restoring it if minimised
			void raise() override {
				switch(state){
					case State::docked:
					case State::floating:
						graphicsDisplay->raise();
					break;
					case State::minimised:
						graphicsDisplay->raise();
						restore();
					break;
				}
			}

			// raise and focus the current widnow, restoring it if minimised
			void focus() override {
				if(focusedWindow==this) return;
				focusedWindow = this;

				//TODO: avoid redrawing twice here
				raise();
				update_focused_window();
			}

			void set_layer(Layer layer) override {
				switch(layer) {
					case Window::Layer::desktopBackground:
						graphicsDisplay->set_layer(DisplayManager::DisplayLayer::background);
					break;
					case Window::Layer::desktop:
						graphicsDisplay->set_layer((DisplayManager::DisplayLayer)((U32)DisplayManager::DisplayLayer::background+1));
					break;
					case Window::Layer::regular:
						graphicsDisplay->set_layer(DisplayManager::DisplayLayer::regular);
					break;
					case Window::Layer::topmost:
						graphicsDisplay->set_layer(DisplayManager::DisplayLayer::topMost);
					break;
				}
			}

			void set_max_docked_size(U32 width, U32 height) {
				maxDockedWidth = width;
				maxDockedHeight = height;

				//TODO: resize if already docked
			}
			
			auto is_top() -> bool {
				return graphicsDisplay->is_top();
			}
			
			void show();
			void hide();

			auto get_state() -> State {
				return state;
			}
			auto get_docked_type() -> DockedType {
				return dockedType;
			}

			virtual void _draw_decorations() {

			}

			virtual void redraw_decorations() {
				_draw_decorations();

				graphicsDisplay->update_area(graphics2d::Rect{0, 0, (I32)leftMargin+get_width()+(I32)rightMargin, (I32)topMargin});
				graphicsDisplay->update_area(graphics2d::Rect{0, (I32)topMargin, (I32)leftMargin, (I32)topMargin+get_height()});
				graphicsDisplay->update_area(graphics2d::Rect{(I32)leftMargin+get_width(), (I32)topMargin, (I32)leftMargin+get_width()+(I32)rightMargin, (I32)topMargin+get_height()});
				graphicsDisplay->update_area(graphics2d::Rect{0, (I32)topMargin+get_height(), (I32)leftMargin+get_width()+(I32)rightMargin, (I32)topMargin+get_height()+(I32)bottomMargin});
			}

			void redraw() {
				graphicsDisplay->update_area({(I32)leftMargin, (I32)topMargin, (I32)leftMargin+get_width(), (I32)topMargin+get_height()});
			}

			void redraw_area(graphics2d::Rect rect) {
				graphicsDisplay->update_area(rect.offset(leftMargin, topMargin));
			}

			virtual void _set_titlebar_area(graphics2d::Rect rect) {
				titlebarArea = rect;
			}

			virtual void _on_mouse_moved(I32 x, I32 y) {}
			virtual void _on_mouse_pressed(I32 x, I32 y, U32 button) {}
			virtual void _on_mouse_released(I32 x, I32 y, U32 button) {}

			DisplayManager::Display *graphicsDisplay;
			const char *title;
			graphics2d::Buffer clientArea;
			graphics2d::Rect titlebarArea;

			Cursor *draggingCursor = nullptr;
			State state = State::floating;

			DockedType dockedType = DockedType::full;
			graphics2d::Rect defaultFloatingRect;

			U32 maxDockedWidth = 1<<31-1;
			U32 maxDockedHeight = 1<<31-1;

			bool _draw_focused = false; // is the window currently draw as the focused/top window? (to avoid excess updates)

			U32 leftMargin = 0;
			U32 topMargin = 0;
			U32 rightMargin = 0;
			U32 bottomMargin = 0;
		};

		void _enable_docked_window(Window&);
		void _disable_docked_window(Window&);

		void Window::move_to(I32 x, I32 y) {
			const auto margin = 10;
			x = maths::clamp(x, -(I32)graphicsDisplay->buffer.width+margin, (I32)displayManager->get_width()-margin);
			y = maths::clamp(y, 0, (I32)displayManager->get_height()-margin);

			graphicsDisplay->move_to(x-leftMargin, y-topMargin, DeferWindowMove::depth<1);
		}

		void Window::resize_to(U32 width, U32 height) {
			auto oldWidth = get_width();
			auto oldHeight = get_height();
			graphicsDisplay->resize_to(leftMargin+width+rightMargin, topMargin+height+bottomMargin, DeferWindowMove::depth<1);
			clientArea = graphicsDisplay->buffer.cropped(leftMargin, topMargin, rightMargin, bottomMargin);

			auto deltaX = (I32)get_width()-(I32)oldWidth;
			auto deltaY = (I32)get_height()-(I32)oldHeight;

			if(deltaX==0&&deltaY==0) return;

			if(enableTransparency){
				graphicsDisplay->buffer.draw_rect((graphics2d::Rect){0,0,(I32)graphicsDisplay->buffer.width,(I32)graphicsDisplay->buffer.height}, 0xff000000);
			}

			graphicsDisplay->solidArea.x2 += max(-(I32)graphicsDisplay->solidArea.width(), deltaX);
			graphicsDisplay->solidArea.y2 += max(-(I32)graphicsDisplay->solidArea.height(), deltaY);
			graphicsDisplay->interactArea.x2 += max(-(I32)graphicsDisplay->interactArea.width(), deltaX);
			graphicsDisplay->interactArea.y2 += max(-(I32)graphicsDisplay->interactArea.height(), deltaY);
			titlebarArea.x2 += max(-(I32)titlebarArea.width(), deltaX);
			titlebarArea.y2 += max(-(I32)titlebarArea.height(), deltaY);

			if(DeferWindowMove::depth<1){
				redraw_decorations();

				events.trigger({
					type: DesktopManager::Window::Event::Type::clientAreaChanged
				});

				redraw();
			}
		}

		void Window::move_and_resize_to(I32 x, I32 y, U32 width, U32 height) {
			DeferWindowMove deferMove(*this);

			move_to(x, y);
			resize_to(width, height);
		}

		void Window::show() {
			graphicsDisplay->show();
			if(state==State::docked){
				_enable_docked_window(*this);
			}
		}
		
		void Window::hide() {
			graphicsDisplay->hide();
			if(state==State::docked){
				_disable_docked_window(*this);
			}
		}

		void Window::dock(DockedType type) {
			switch(state){
				case State::docked:
					if(dockedType==type) return;

					_disable_docked_window(*this);
				break;
				case State::floating:
					defaultFloatingRect = {get_x(), get_y(), get_x()+get_width(), get_y()+get_height()};
				break;
				case State::minimised:
					graphicsDisplay->show();
					defaultFloatingRect = {get_x(), get_y(), get_x()+get_width(), get_y()+get_height()};
				break;
			}

			state = State::docked;
			dockedType = type;

			auto windowArea = DesktopManager::instance.get_window_area();

			switch(dockedType){
				case DockedType::top:
					move_and_resize_to(
						windowArea.x1, windowArea.y1,
						windowArea.width(), max(16, min(windowArea.height()/2-4, (I32)maxDockedHeight))
					);
				break;
				case DockedType::bottom: {
					const auto height = max(16, min(windowArea.height()/2-4, (I32)maxDockedHeight));
					move_and_resize_to(
						windowArea.x1, windowArea.y2-height,
						windowArea.width(), height
					);
				} break;
				case DockedType::left:
					move_and_resize_to(
						windowArea.x1, windowArea.y1,
						max(16, min(windowArea.width()/2-4, (I32)maxDockedWidth)), windowArea.height()
					);
				break;
				case DockedType::right: {
					const auto width = max(16, min(windowArea.width()/2-4, (I32)maxDockedWidth));
					move_and_resize_to(
						windowArea.x2-width, windowArea.y1,
						width, windowArea.height()
					);
				} break;
				case DockedType::full:
					move_and_resize_to(
						windowArea.x1, windowArea.y1,
						windowArea.width(), windowArea.height()
					);
				break;
			}

			_enable_docked_window(*this);
		}

		void Window::minimise() {
			switch(state){
				case State::docked:
					_disable_docked_window(*this);

					graphicsDisplay->hide();
					move_and_resize_to(
						defaultFloatingRect.x1, defaultFloatingRect.y1,
						defaultFloatingRect.width(), defaultFloatingRect.height()
					);
					state = State::minimised;
				break;
				case State::floating:
					graphicsDisplay->hide();
					state = State::minimised;
				break;
				case State::minimised:
					return;
				break;
			}

			if(focusedWindow==this){
				focusedWindow = nullptr;
				update_focused_window();
			}
		}

		void Window::restore() {
			switch(state){
				case State::docked: {
					state = State::floating;
					_disable_docked_window(*this);

					move_and_resize_to(
						defaultFloatingRect.x1, defaultFloatingRect.y1,
						defaultFloatingRect.width(), defaultFloatingRect.height()
					);
				}break;
				case State::floating:
					return;
				break;
				case State::minimised:
					state = State::floating;
					graphicsDisplay->show();
					return;
				break;
			}
		}

		/**/ DeferWindowMove:: DeferWindowMove(Window &window):
			window(window),
			oldRect{window.graphicsDisplay->x, window.graphicsDisplay->y, window.graphicsDisplay->x+(I32)window.graphicsDisplay->get_width(), window.graphicsDisplay->y+(I32)window.graphicsDisplay->get_height()},
			oldBuffer(window.graphicsDisplay->buffer.address),
			oldBufferWidth(window.graphicsDisplay->buffer.width),
			oldBufferHeight(window.graphicsDisplay->buffer.height)
		{
			depth++;
		}

		/**/ DeferWindowMove::~DeferWindowMove(){
			auto newRect = (graphics2d::Rect){window.graphicsDisplay->x, window.graphicsDisplay->y, window.graphicsDisplay->x+(I32)window.graphicsDisplay->get_width(), window.graphicsDisplay->y+(I32)window.graphicsDisplay->get_height()};

			auto changed = false;

			if(newRect!=oldRect) {
				changed = true;
				displayManager->update_area(oldRect, window.graphicsDisplay);
			}

			if(window.graphicsDisplay->buffer.address!=oldBuffer||window.graphicsDisplay->buffer.width!=oldBufferWidth||window.graphicsDisplay->buffer.height!=oldBufferHeight){
				changed = true;

				window._draw_decorations();

				window.events.trigger({
					type: DesktopManager::Window::Event::Type::clientAreaChanged
				});
			}

			if(changed){
				window.graphicsDisplay->update();
			}

			depth--;
		}

		struct StandardWindow: Window, DesktopManager::StandardWindow {
			typedef driver::Window Super;

			ui2d::control::ColouredButton closeButton;
			ui2d::control::ColouredButton maximiseButton;
			struct MinimiseButton: ui2d::control::ColouredButton {
				/**/ MinimiseButton(): ui2d::control::ColouredButton(window().gui, {0,0,0,0}, 0xffff00, "") {
					icon = &ui2d::image::widgets::minimise;
				}

				void on_clicked() override {
					window().minimise();
				};

				auto window() -> StandardWindow& { return parent(*this, &StandardWindow::minimiseButton); }
			} minimiseButton;

			static const auto widgetSpacing = 4;

			static const auto cornerRadius = enableTransparency?5:2;
			U32 corner[cornerRadius+1];
			U32 cornerInner[cornerRadius-1+1];

			static const U32 leftShadow = enableTransparency?8:0;
			static const U32 rightShadow = enableTransparency?8:0;
			static const U32 topShadow = enableTransparency?8:0;
			static const U32 bottomShadow = enableTransparency?8:0;

			static const U8 shadowIntensity = 20; // max intensity
			static const U8 topShadowIntensity = 128; // scaling down of top shadow (by inner extension)
			static const U8 leftShadowIntensity = 192; // scaling down of left shadow (by inner extension)
			static const U8 rightShadowIntensity = 192; // scaling down of right shadow (by inner extension)

			const char *status = "";

			/**/ StandardWindow(const char *title, U32 width, U32 height):
				Super(title, width+leftShadow+rightShadow, height+topShadow+bottomShadow),
				closeButton(gui, {0,0,0,0}, 0xff0000, ""),
				maximiseButton(gui, {0,0,0,0}, 0xff8800, "")
			{
				leftMargin = leftShadow;
				topMargin = topShadow;
				rightMargin = rightShadow;
				bottomMargin = bottomShadow;

				closeButton.icon = &ui2d::image::widgets::close;
				maximiseButton.icon = &ui2d::image::widgets::maximise;

				if(graphicsDisplay){
					clientArea = graphicsDisplay->buffer.cropped(leftMargin, topMargin, rightMargin, bottomMargin);
					gui.buffer = graphicsDisplay->buffer;
					if(enableTransparency){
						graphicsDisplay->buffer.draw_rect((graphics2d::Rect){0,0,(I32)graphicsDisplay->buffer.width,(I32)graphicsDisplay->buffer.height}, 0xff000000);
					}
				}

				_set_titlebar_area({(I32)leftShadow, (I32)topShadow, (I32)leftShadow+(I32)width, (I32)topShadow+titlebarHeight});

				// graphics2d::create_round_corner(cornerRadius, corner);
				graphics2d::create_diagonal_corner(cornerRadius, corner);
				graphics2d::create_diagonal_corner(cornerRadius-1, cornerInner);

				memcpy(graphicsDisplay->topLeftCorner, corner, sizeof(corner)-sizeof(corner[0]));
				memcpy(graphicsDisplay->topRightCorner, corner, sizeof(corner)-sizeof(corner[0]));
				memcpy(graphicsDisplay->bottomLeftCorner, corner, sizeof(corner)-sizeof(corner[0]));
				memcpy(graphicsDisplay->bottomRightCorner, corner, sizeof(corner)-sizeof(corner[0]));
			}

			auto get_background_colour() -> U32 override {
				return windowBackgroundColour;
			}

			auto get_border_colour() -> U32 override {
				return windowBorderColour;
			}

			auto get_border_rect() -> graphics2d::Rect {
				return graphics2d::Rect{(I32)leftMargin, (I32)topMargin, (I32)leftMargin+get_width(), (I32)topMargin+get_height()};
			}

			void set_status(const char*) override;

			void resize_to(U32 width, U32 height) override;

			void redraw() override;
			void redraw_area(graphics2d::Rect) override;

			void _set_titlebar_area(graphics2d::Rect rect) override {
				Super::_set_titlebar_area(rect);

				const auto size = titlebarArea.height()-widgetSpacing*2;

				closeButton.rect = {titlebarArea.x2-widgetSpacing-size, titlebarArea.y1+widgetSpacing, titlebarArea.x2-widgetSpacing, titlebarArea.y2-widgetSpacing};
				maximiseButton.rect = {closeButton.rect.x1-widgetSpacing-1-widgetSpacing-size, titlebarArea.y1+widgetSpacing, closeButton.rect.x1-widgetSpacing-1-widgetSpacing, titlebarArea.y2-widgetSpacing};
				minimiseButton.rect = {maximiseButton.rect.x1-widgetSpacing-size, titlebarArea.y1+widgetSpacing, maximiseButton.rect.x1-widgetSpacing, titlebarArea.y2-widgetSpacing};

				gui.redraw(false);
			}

			void _draw_decorations() override {
				auto rect = get_border_rect();
				const auto borderColour = draggingCursor?0xd0b0b0:windowBorderColour;
				const auto titlebarBgColour = draggingCursor?0xfff9f9:_draw_focused?0xf9f9f9:windowBackgroundColour;
				const auto titlebarTextColour = _draw_focused?0x333333:0x999999;
				const auto statusbarTextColour = _draw_focused?0x666666:0xaaaaaa;

				U32 innerAaCorner[2+1];
				graphics2d::create_diagonal_corner(2, innerAaCorner);

				if(enableTransparency){
					{ // draw border minus corners
						graphicsDisplay->buffer.draw_line(rect.x1+cornerRadius, rect.y1, rect.x2-1-cornerRadius, rect.y1, borderColour);
						graphicsDisplay->buffer.draw_line(rect.x1+cornerRadius, rect.y2-1, rect.x2-1-cornerRadius, rect.y2-1, borderColour);
						graphicsDisplay->buffer.draw_line(rect.x1, rect.y1+cornerRadius, rect.x1, rect.y2-cornerRadius, borderColour);
						graphicsDisplay->buffer.draw_line(rect.x2-1, rect.y1+cornerRadius, rect.x2-1, rect.y2-cornerRadius, borderColour);
					}

				}else{
					graphicsDisplay->buffer.draw_rect_outline(rect.x1, rect.y1, get_width(), get_height(), borderColour, 1, corner, corner, corner, corner);
					graphicsDisplay->buffer.draw_rect(rect.x1+1, rect.y1+1, get_width()-2, titlebarHeight-2, titlebarBgColour, corner, corner, nullptr, nullptr);
				}

				{ // draw titlebar divide
					graphicsDisplay->buffer.draw_line(rect.x1+1, rect.y1+titlebarHeight-1, rect.x2-2, rect.y1+titlebarHeight-1, _draw_focused?borderColour:windowBackgroundColour);
				}

				{ // draw statusbar divide
					graphicsDisplay->buffer.draw_line(rect.x1+1, rect.y1+get_height()-1-21-1, rect.x2-2, rect.y1+get_height()-statusbarHeight, _draw_focused?0xcccccc:0xdddddd);
				}

				if(enableTransparency){
					{ // undraw any previous corners with transparency, otherwise we get line doubling on the aa pixels
						graphicsDisplay->buffer.draw_rect(rect.x1, rect.y1, cornerRadius, cornerRadius, 0xff000000);
						graphicsDisplay->buffer.draw_rect(rect.x2-cornerRadius, rect.y1, cornerRadius, cornerRadius, 0xff000000);
						graphicsDisplay->buffer.draw_rect(rect.x1, rect.y2-cornerRadius, cornerRadius, cornerRadius, 0xff000000);
						graphicsDisplay->buffer.draw_rect(rect.x2-cornerRadius, rect.y2-cornerRadius, cornerRadius, cornerRadius, 0xff000000);

						// put shadows under corners
						for(auto y=0;y<cornerRadius;y++){
							auto width = 1+cornerRadius-1-y;
							for(auto x=0;x<width;x++){
								graphicsDisplay->buffer.set(rect.x1+x, rect.y1+y, 0x000000|(255-(get_shadow_intensity_at(rect.x1+x, rect.y1+y)*shadowIntensity/255)<<24));
								graphicsDisplay->buffer.set(rect.x2-1-x, rect.y1+y, 0x000000|(255-(get_shadow_intensity_at(rect.x2-1-x, rect.y1+y)*shadowIntensity/255)<<24));
								graphicsDisplay->buffer.set(rect.x1+x, rect.y2-1-y, 0x000000|(255-(get_shadow_intensity_at(rect.x1+x, rect.y2-1-y)*shadowIntensity/255)<<24));
								graphicsDisplay->buffer.set(rect.x2-1-x, rect.y2-1-y, 0x000000|(255-(get_shadow_intensity_at(rect.x2-1-x, rect.y2-1-y)*shadowIntensity/255)<<24));
							}
						}
					}

					{ // draw titlebar block
						if(_draw_focused){
							graphicsDisplay->buffer.draw_rect(rect.x1+1, rect.y1+1, get_width()-2, titlebarHeight-2, titlebarBgColour, innerAaCorner, innerAaCorner, nullptr, nullptr);
						}else{
							graphicsDisplay->buffer.draw_rect(rect.x1+1, rect.y1+1, get_width()-2, titlebarHeight-2, titlebarBgColour, innerAaCorner, innerAaCorner, nullptr, nullptr);
							// for(auto y=0;y<titlebarHeight-2;y++){
							// 	graphicsDisplay->buffer.set(rect.x1+1+graphicsDisplay->get_left_margin(rect.y1+1+y), (U32)rect.y1+1+y, graphics2d::blend_rgb(0xf9f9f9, titlebarBgColour, min(1.0f, y/(float)(titlebarHeight/4-3))), get_width()-2-graphicsDisplay->get_left_margin(rect.y1+1+y)-graphicsDisplay->get_right_margin(rect.y1+1+y));
							// }
						}
					}

					{ // draw statusbar block
						graphicsDisplay->buffer.draw_rect(rect.x1+1, rect.y2-statusbarHeight+1, get_width()-2, statusbarHeight-2, windowBackgroundColour, nullptr, nullptr, innerAaCorner, innerAaCorner);
					}

					{ // draw aa corners
						for(auto y=0;y<5;y++){
							for(auto x=0;x<5;x++){
								graphicsDisplay->buffer.set_blended(rect.x1+x, rect.y1+y, graphics2d::premultiply_colour((borderColour&0x00ffffff)|((255-corner5x5Graphic[y*5+x])<<24)));
								graphicsDisplay->buffer.set_blended(rect.x2-1-x, rect.y1+y, graphics2d::premultiply_colour((borderColour&0x00ffffff)|((255-corner5x5Graphic[y*5+x])<<24)));
								graphicsDisplay->buffer.set_blended(rect.x1+x, rect.y2-1-y, graphics2d::premultiply_colour((borderColour&0x00ffffff)|((255-corner5x5Graphic[y*5+x])<<24)));
								graphicsDisplay->buffer.set_blended(rect.x2-1-x, rect.y2-1-y, graphics2d::premultiply_colour((borderColour&0x00ffffff)|((255-corner5x5Graphic[y*5+x])<<24)));
							}
						}
					}

				}else{
					{ // draw titlebar block
						graphicsDisplay->buffer.draw_rect(rect.x1+1, rect.y1+1, get_width()-2, titlebarHeight-2, titlebarBgColour, cornerInner, cornerInner, nullptr, nullptr);
					}

					{ // draw statusbar block
						graphicsDisplay->buffer.draw_rect(rect.x1+1, rect.y2-statusbarHeight+1, get_width()-2, statusbarHeight-2, windowBackgroundColour, nullptr, nullptr, cornerInner, cornerInner);
					}
				}

				{ // draw titlebar text
					const auto textSize = graphicsDisplay->buffer.measure_text({.font=*graphics2d::font::default_sans, .size=14}, title, rect.width());
					graphicsDisplay->buffer.draw_text({.font=*graphics2d::font::default_sans, .size=14}, title, rect.x1+(rect.width()-textSize.blockWidth)/2, rect.y1+(titlebarHeight-2)/2+textSize.capHeight-textSize.blockHeight/2, rect.width(), titlebarTextColour);
				}

				closeButton.opacity = _draw_focused?0xff:0x66;
				maximiseButton.opacity = _draw_focused?0xff:0x66;
				minimiseButton.opacity = _draw_focused?0xff:0x66;

				{ // draw widget divide
					graphicsDisplay->buffer.draw_line(closeButton.rect.x1-widgetSpacing-1, titlebarArea.y1+widgetSpacing+1, closeButton.rect.x1-widgetSpacing-1, titlebarArea.y2-widgetSpacing-1, graphics2d::blend_colours(titlebarBgColour, _draw_focused?0xdd000000:0xee000000));
				}

				gui.redraw();

				{ // draw statusbar text
					// auto lineHeight = 14*5/4;
					graphicsDisplay->buffer.draw_text({.font=*graphics2d::font::default_sans, .size=14}, status, rect.x1+4, rect.y2-7, get_width(), statusbarTextColour);
				}
			}

			void redraw_decorations() override {
				auto rect = get_border_rect();

				_draw_decorations();
				//top
				// graphicsDisplay->update_area(graphics2d::Rect{0, 0, (I32)get_width(), cornerRadius}.offset(rect.x1, rect.y1));
				graphicsDisplay->update_area(graphics2d::Rect{0, 0, (I32)get_width(), titlebarHeight}.offset(rect.x1, rect.y1));
				//left
				graphicsDisplay->update_area(graphics2d::Rect{0, cornerRadius, 1, (I32)get_height()-cornerRadius}.offset(rect.x1, rect.y1));
				//right
				graphicsDisplay->update_area(graphics2d::Rect{(I32)get_width()-1, cornerRadius, (I32)get_width(), (I32)get_height()-cornerRadius}.offset(rect.x1, rect.y1));
				//bottom
				// graphicsDisplay->update_area(graphics2d::Rect{0, (I32)get_height()-1, (I32)get_width(), (I32)get_height()}.offset(rect.x1, rect.y1));
				graphicsDisplay->update_area(graphics2d::Rect{0, (I32)get_height()-statusbarHeight, (I32)get_width(), (I32)get_height()}.offset(rect.x1, rect.y1));
			}

			static const auto titlebarHeight = 30;
			static const auto statusbarHeight = 23;

			void draw_frame() {
				// auto rect = get_border_rect();

				// U32 corner[4] = {3, 1, 1, (U32)~0};
				// U32 cornerInner[3] = {2, 1, (U32)~0};

				auto &display = *graphicsDisplay;

				_draw_decorations();
				_draw_shadow();

				clientArea = display.buffer.cropped(leftShadow+1, topShadow+titlebarHeight, rightShadow+1, bottomShadow+23);
				clientArea.draw_rect(0, 0, clientArea.width, clientArea.height, windowBackgroundColour);
				gui.redraw(false);
				display.update();
			}

			auto get_shadow_intensity_at(I32 x, I32 y) -> U8 {
				U8 intensity = 255;

				//we lengthen it inward when the intensity is turned down (so it fades in nicely at the bottom)
				auto topShadowLength = (I32)topShadow * 255/(I32)topShadowIntensity;
				auto leftShadowLength = (I32)leftShadow * 255/(I32)leftShadowIntensity;
				auto rightShadowLength = (I32)rightShadow * 255/(I32)rightShadowIntensity;

				if(x<(I32)leftShadowLength&&x<(I32)graphicsDisplay->get_width()/2){
					intensity = intensity * (x+1)/leftShadowLength;

				}else if(x>=(I32)graphicsDisplay->get_width()-rightShadowLength&&x>=(I32)graphicsDisplay->get_width()/2){
					intensity = intensity * (leftShadow+get_width()+rightShadow-x)/rightShadowLength;
				}

				{
					if(y<(I32)topShadow){
						intensity = intensity * (y+1)/topShadowLength;

					}else if(y>=(I32)topShadow+get_height()){
						intensity = intensity * (topShadow+get_height()+bottomShadow-y)/bottomShadow;
					}
				}

				// const auto topShadow = bottomShadow*2; // top fade twice of bottom

				// if(y<(I32)topShadow){
				// 	// use for top fade, as well
				// 	intensity = intensity * (y+1)/topShadow;

				// }else if(y>=get_height()){
				// 	intensity = intensity * (get_height()+bottomShadow-y)/bottomShadow;
				// }

				return intensity;
			}

			void _draw_shadow() {
				if(!enableTransparency) return;

				for(auto i=0u;i<sizeof(graphicsDisplay->topLeftCorner)/sizeof(graphicsDisplay->topLeftCorner[0]);i++){
					graphicsDisplay->topLeftCorner[i] = 0;
					graphicsDisplay->topRightCorner[i] = 0;
					graphicsDisplay->bottomLeftCorner[i] = 0;
					graphicsDisplay->bottomRightCorner[i] = 0;
				}
				graphicsDisplay->solidArea = {(I32)leftShadow+cornerRadius, (I32)topShadow, (I32)leftShadow+get_width()-cornerRadius, (I32)topShadow+get_height()-(I32)bottomShadow};
				graphicsDisplay->interactArea = get_border_rect();

				auto &buffer = graphicsDisplay->buffer;
				for(auto y=0; y<(I32)buffer.height; y++){
					for(auto x=0; x<(I32)leftShadow; x++){
						buffer.set(x, y, 0x000000|(255-(get_shadow_intensity_at(x, y)*shadowIntensity/255)<<24));
					}
					for(auto x=0; x<(I32)rightShadow; x++){
						buffer.set((I32)leftShadow+(I32)get_width()+(I32)rightShadow-1-x, y, 0x000000|(255-(get_shadow_intensity_at(x, y)*shadowIntensity/255)<<24));
					}
				}

				for(auto y=0; y<(I32)topShadow; y++){
					for(auto x=0;x<get_width();x++){
						buffer.set((I32)leftShadow+x, y, 0x000000|(255-(get_shadow_intensity_at(leftShadow+x, y)*shadowIntensity/255)<<24));
					}
				}

				for(auto y=0; y<(I32)bottomShadow; y++){
					// buffer.set((I32)leftShadow, get_height()+y, 0x000000|(255-(get_shadow_intensity_at(leftShadow, get_height()+y)*shadowIntensity/255)<<24), get_width());

					for(auto x=0;x<get_width();x++){
						buffer.set((I32)leftShadow+x, (I32)topShadow+get_height()+y, 0x000000|(255-(get_shadow_intensity_at(leftShadow+x, (I32)topShadow+get_height()+y)*shadowIntensity/255)<<24));
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

				graphicsDisplay->update_area({(I32)leftShadow, 0, (I32)buffer.width-(I32)rightShadow, (I32)topShadow});
				graphicsDisplay->update_area({0, 0, (I32)leftShadow, (I32)buffer.height});
				graphicsDisplay->update_area({(I32)leftShadow+get_width(), 0, (I32)leftShadow+get_width()+(I32)rightShadow, (I32)buffer.height});
				graphicsDisplay->update_area({(I32)leftShadow, (I32)topShadow+get_height(), get_width(), (I32)topShadow+get_height()+(I32)bottomShadow});
			}

			void _on_mouse_moved(I32 x, I32 y) override {
				gui.on_mouse_moved(x, y);
			}
			void _on_mouse_pressed(I32 x, I32 y, U32 button) override {
				gui.on_mouse_pressed(x, y, button);
			}
			void _on_mouse_released(I32 x, I32 y, U32 button) override {
				gui.on_mouse_released(x, y, button);
			}
		};

		struct CustomWindow: Window, DesktopManager::CustomWindow {
			typedef Window Super;

			/**/ CustomWindow(const char *title, U32 width, U32 height):
				Super(title, width, height)
			{
				if(graphicsDisplay){
					clientArea = graphicsDisplay->buffer;
					if(enableTransparency){
						graphicsDisplay->buffer.draw_rect((graphics2d::Rect){0,0,(I32)graphicsDisplay->buffer.width,(I32)graphicsDisplay->buffer.height}, 0xff000000);
					}
				}
			}

			auto get_window_area() -> graphics2d::Buffer& override {
				return graphicsDisplay->buffer;
			}

			void set_titlebar_area(graphics2d::Rect set) override {
				titlebarArea = set;
			}

			void set_solid_area(graphics2d::Rect set) override {
				graphicsDisplay->solidArea = set;
			}

			void set_interact_area(graphics2d::Rect set) override {
				graphicsDisplay->interactArea = set;
			}

			void set_margin(U32 left, U32 top, U32 right, U32 bottom) override {
				const auto oldX = get_x();
				const auto oldY = get_y();

				leftMargin = left;
				topMargin = top;
				rightMargin = right;
				bottomMargin = bottom;

				clientArea = graphicsDisplay->buffer.cropped(leftMargin, topMargin, rightMargin, bottomMargin);
				move_to(oldX, oldY);
				//TODO: handle any resize repurcusions (like docked windows)
			}
		};

		auto get_window_at(I32 x, I32 y, DisplayManager::Display *below = nullptr) -> Window* {
			auto display = displayManager->get_display_at(x, y, false, below);
			auto windowInterface = display?DesktopManager::instance.get_window_from_display(*display):nullptr;
			auto window = windowInterface?(Window*)(StandardWindow*)windowInterface->as_standardWindow()?:(Window*)(CustomWindow*)windowInterface->as_customWindow():nullptr;

			return window;
		}

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
				if(window.is_top()){
					window.redraw_decorations();

				}else{
					window._draw_decorations();
					window.raise(); // this will trigger a redraw, so we won't use the redraw_* variants
					focusedWindow = &window;
					update_focused_window();
				}
			}

			void release_window() {
				if(!dragWindow.window||dragWindow.window->draggingCursor!=this) return;

				dragWindow.window->draggingCursor = nullptr;
				dragWindow.window->redraw_decorations();
				dragWindow.window = nullptr;
			}

			void regrab_window() {
				dragWindow.dragOffsetX = maths::clamp(dragWindow.dragOffsetX, -(I32)dragWindow.window->titlebarArea.x2+8, -(I32)dragWindow.window->titlebarArea.x1-8);
				dragWindow.dragOffsetY = maths::clamp(dragWindow.dragOffsetY, -(I32)dragWindow.window->titlebarArea.y2+8, -(I32)dragWindow.window->titlebarArea.y1-8);
			}
		};

		LList<Window> windows;
		Window *focusedWindow = nullptr;
		ListUnordered<Cursor> cursors(0); // do not preallocate any (dynamic allocations on boot fail before memory is setup)

		void _add_mouse(Mouse &mouse);
		void _remove_mouse(Mouse &mouse);
		auto _find_cursor(Mouse &mouse) -> Cursor*;

		void _on_drivers_event(const drivers::Event &event) {
			switch(event.type){
				case drivers::Event::Type::driverStarted:
					if(auto mouse = event.driverStarted.driver->as_type<Mouse>()){
						_add_mouse(*mouse);
					}
				break;
				case drivers::Event::Type::driverStopped:
					if(auto mouse = event.driverStopped.driver->as_type<Mouse>()){
						_remove_mouse(*mouse);
					}
				break;
			}
		}

		void _add_mouse(Mouse &mouse) {
			if(!displayManager) return;

			auto &log = DesktopManager::instance.log;

			log.print_info("mouse added");

			auto x = (I32)displayManager->get_width()/2;
			auto y = (I32)displayManager->get_height()/2;
			auto &cursorImage = ui2d::image::cursors::_default;

			auto cursorDisplay = displayManager->create_display(nullptr, DisplayManager::DisplayLayer::cursor, cursorImage.width, cursorImage.height);
			// cursorDisplay->mode = DisplayManager::DisplayMode::transparent; // TODO: add some kind of api for specifically marking displays as transparent
			cursorDisplay->move_to(x, y);
			cursorDisplay->solidArea.clear();
			cursorDisplay->buffer.draw_buffer(0, 0, 0, 0, cursorImage.width, cursorImage.height, cursorImage);

			cursors.push({&mouse, cursorDisplay, x, y, false});
		}

		void _remove_mouse(Mouse &mouse) {
			auto &log = DesktopManager::instance.log;

			for(auto i=0u;i<cursors.length;i++){
				auto &cursor = cursors[i];
				if(cursor.mouse==&mouse){
					log.print_info("mouse removed");
					delete cursor.display;
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

		void _update_window_area(Window *exclude = nullptr);

		void _enable_docked_window(Window &window){
			_update_window_area();

			//TODO: adjust non-docked windows sitting outside window area
		}

		void _disable_docked_window(Window &dockedWindow){
			for(auto window=windows.head; window; window=window->next){
				switch(window->state){
					case DesktopManager::Window::State::docked:
						switch(dockedWindow.dockedType){
							case DesktopManager::Window::DockedType::left:
								switch(window->dockedType){
									case DesktopManager::Window::DockedType::top:
									case DesktopManager::Window::DockedType::bottom:
									case DesktopManager::Window::DockedType::full:
										if(window->get_x()>=dockedWindow.get_x()+dockedWindow.get_width()){
											window->move_and_resize_to(window->get_x()-dockedWindow.get_width(), window->get_y(), window->get_width()+dockedWindow.get_width(), window->get_height());
										}
									break;
									case DesktopManager::Window::DockedType::left:
										if(window->get_x()>=dockedWindow.get_x()+dockedWindow.get_width()){
											window->move_to(window->get_x()-dockedWindow.get_width(), window->get_y());
										}
									break;
									case DesktopManager::Window::DockedType::right:
									break;
								}
							break;
							case DesktopManager::Window::DockedType::right:
								switch(window->dockedType){
									case DesktopManager::Window::DockedType::top:
									case DesktopManager::Window::DockedType::bottom:
									case DesktopManager::Window::DockedType::full:
										if(window->get_x()+window->get_width()<=dockedWindow.get_x()){
											window->resize_to(window->get_width()+dockedWindow.get_width(), window->get_height());
										}
									break;
									case DesktopManager::Window::DockedType::left:
									break;
									case DesktopManager::Window::DockedType::right:
										if(window->get_x()+window->get_width()<=dockedWindow.get_x()){
											window->move_to(window->get_x()+dockedWindow.get_width(), window->get_y());
										}
									break;
								}
							break;
							case DesktopManager::Window::DockedType::top:
								switch(window->dockedType){
									case DesktopManager::Window::DockedType::left:
									case DesktopManager::Window::DockedType::right:
									case DesktopManager::Window::DockedType::full:
										if(window->get_y()>=dockedWindow.get_y()+dockedWindow.get_height()){
											window->move_and_resize_to(window->get_x(), window->get_y()-dockedWindow.get_height(), window->get_width(), window->get_height()+dockedWindow.get_height());
										}
									break;
									case DesktopManager::Window::DockedType::top:
										if(window->get_y()>=dockedWindow.get_y()+dockedWindow.get_height()){
											window->move_to(window->get_x(), window->get_y()-dockedWindow.get_height());
										}
									break;
									case DesktopManager::Window::DockedType::bottom:
									break;
								}
							break;
							case DesktopManager::Window::DockedType::bottom:
								switch(window->dockedType){
									case DesktopManager::Window::DockedType::left:
									case DesktopManager::Window::DockedType::right:
									case DesktopManager::Window::DockedType::full:
										if(window->get_y()+window->get_height()<=dockedWindow.get_y()){
											window->resize_to(window->get_width(), window->get_height()+dockedWindow.get_height());
										}
									break;
									case DesktopManager::Window::DockedType::top:
									break;
									case DesktopManager::Window::DockedType::bottom:
										if(window->get_y()+window->get_height()<=dockedWindow.get_y()){
											window->move_to(window->get_x(), window->get_y()+dockedWindow.get_height());
										}
									break;
								}
							break;
							case DesktopManager::Window::DockedType::full:
							break;
						}
					break;
					case DesktopManager::Window::State::floating:
					case DesktopManager::Window::State::minimised:
					break;
				}
			}

			_update_window_area(&dockedWindow);
		}

		// update the window area when floats have changed, moving windows as neccasary to fit in the space
		void _update_window_area(Window *exclude){
			windowArea = {0, 0, (I32)displayManager->get_width(), (I32)displayManager->get_height()};
			for(auto window=windows.head; window; window=window->next){
				if(window==exclude) continue;

				if(window->graphicsDisplay->isVisible&&window->state==Window::State::docked){
					switch(window->dockedType){
						case Window::DockedType::top:
							windowArea = windowArea.intersect({0, window->get_y()+window->get_height(), (I32)displayManager->get_width(), (I32)displayManager->get_height()});
						break;
						case Window::DockedType::bottom:
							windowArea = windowArea.intersect({0, 0, (I32)displayManager->get_width(), window->get_y()});
						break;
						case Window::DockedType::left:
							windowArea = windowArea.intersect({window->get_x()+window->get_width(), 0, (I32)displayManager->get_width(), (I32)displayManager->get_height()});
						break;
						case Window::DockedType::right:
							windowArea = windowArea.intersect({0, 0,  window->get_x(), (I32)displayManager->get_height()});
						break;
						case Window::DockedType::full:
						break;
					}
				}
			}

			//TODO: move windows overlapping outside windowArea

			for(auto window=windows.head; window; window=window->next){
				if(window==exclude) continue;
				switch(window->state){
					case Window::State::floating: {
						auto area = (graphics2d::Rect){window->get_x(), window->get_y(), window->get_x()+window->get_width(), window->get_y()+window->get_height()};

						if(area.width()>=windowArea.width()){
							area.x1 = windowArea.x1;
							area.x2 = windowArea.x2;

						}else{
							auto margin = (windowArea.width()-area.width())/2;

							if(area.x1<windowArea.x1){
								area = area.offset(min((windowArea.x2-area.x2)-margin, (windowArea.x1-area.x1)*2), 0); // nudge twice the required distance, but not past the centre of the screen (spaces things nicely on adjust)

							}else if(area.x2>windowArea.x2){
								area = area.offset(max(margin-(area.x1-windowArea.x1), (windowArea.x2-area.x2)*2), 0);
							}
						}

						if(area.height()>=windowArea.height()){
							area.y1 = windowArea.y1;
							area.y2 = windowArea.y2;

						}else{
							auto margin = (windowArea.height()-area.height())/2;

							if(area.y1<windowArea.y1){
								area = area.offset(0, min((windowArea.y2-area.y2)-margin, (windowArea.y1-area.y1)*2));

							}else if(area.y2>windowArea.y2){
								area = area.offset(0, max(margin-(area.y1-windowArea.y1), (windowArea.y2-area.y2)*2));
							}
						}

						window->move_and_resize_to(area.x1, area.y1, area.width(), area.height());
					} break;
					case Window::State::docked:
					break;
					case Window::State::minimised:
					break;
				}
			}
		}

		auto client_to_graphics_area(StandardWindow &window, graphics2d::Rect rect) {
			auto &display = *window.graphicsDisplay;

			const auto addressOffset = window.clientArea.address - display.buffer.address;
			auto yOffset = addressOffset / display.buffer.stride;
			auto xOffset = (addressOffset - (yOffset * display.buffer.stride)) / graphics2d::bufferFormat::size[(U32)display.buffer.format];

			return rect.offset(xOffset, yOffset);
		}

		void update_focused_window() {
			for(auto window=windows.head; window; window=window->next){
				auto isFocused = focusedWindow==window;
				if(isFocused!=window->_draw_focused){
					window->_draw_focused = isFocused;
					window->redraw_decorations();
				}
			}

			DesktopManager::instance.events.trigger({
				type: DesktopManager::Event::Type::windowFocused,
				windowFocused: { window: focusedWindow }
			});	
		}
	}

	namespace {
		void on_keyboard_event(const driver::Keyboard::Event &event) {
			if(focusedWindow){
				switch(event.type){
					case driver::Keyboard::Event::Type::pressed:
						// cancel all window drags on escape
						if(event.pressed.scancode==(keyboard::Scancode)keyboard::ScancodeUk::escape){
							for(auto &cursor:cursors){
								cursor.release_window();
							}
						}
						focusedWindow->events.trigger({
							type: StandardWindow::Event::Type::keyPressed,
							keyPressed: { event.instance, event.pressed.scancode, event.pressed.repeat, event.pressed.modifiers }
						});
					break;
					case driver::Keyboard::Event::Type::released:
						focusedWindow->events.trigger({
							type: StandardWindow::Event::Type::keyReleased,
							keyPressed: { event.instance, event.released.scancode }
						});
					break;
					case driver::Keyboard::Event::Type::actionPressed:
						focusedWindow->events.trigger({
							type: StandardWindow::Event::Type::actionPressed,
							actionPressed: { event.instance, event.actionPressed.action }
						});
					break;
					case driver::Keyboard::Event::Type::characterTyped:
						focusedWindow->events.trigger({
							type: StandardWindow::Event::Type::characterTyped,
							characterTyped: { event.instance, event.characterTyped.character }
						});
					break;
				}
			}
		}
		void on_mouse_event(const driver::Mouse::Event &event) {
			auto cursor = _find_cursor(*event.instance);
			if(!cursor) return;

			auto window = get_window_at(cursor->x, cursor->y, cursor->display);

			// update cursor
			switch(event.type){
				case Mouse::Event::Type::pressed:
					if(window) {
						window->raise();
						focusedWindow = window;
						update_focused_window();
					}else{
						focusedWindow = nullptr;
						update_focused_window();
					}

					if(event.pressed.button==0){
						{
							auto &cursorImage = ui2d::image::cursors::_default_left;
							cursor->display->buffer.draw_buffer(0, 0, 0, 0, cursorImage.width, cursorImage.height, cursorImage);
							cursor->display->update();
						}
						if(!cursor->dragWindow.window){
							if(window&&window->titlebarArea.contains(cursor->x-window->get_x(), cursor->y-window->get_y())){
								cursor->grab_window(*window);
							}
						}

					}else if(event.pressed.button==1){
						{
							auto &cursorImage = ui2d::image::cursors::_default_right;
							cursor->display->buffer.draw_buffer(0, 0, 0, 0, cursorImage.width, cursorImage.height, cursorImage);
							cursor->display->update();
						}
					}
				break;
				case Mouse::Event::Type::released:
					if(event.released.button==0||event.released.button==1){
						auto &cursorImage = ui2d::image::cursors::_default;
						cursor->display->buffer.draw_buffer(0, 0, 0, 0, cursorImage.width, cursorImage.height, cursorImage);
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
						auto windowArea = DesktopManager::instance.get_window_area();

						if(cursor->x<(windowArea.x1+windowArea.x2)/2&&cursor->x<windowArea.x1+(I32)edgeSnapDistance){
							cursor->dragWindow.window->dock(DesktopManager::Window::DockedType::left);
						}else if(cursor->x>=windowArea.x2-(I32)edgeSnapDistance){
							cursor->dragWindow.window->dock(DesktopManager::Window::DockedType::right);
						}else if(cursor->y<(windowArea.y1+windowArea.y2)/2&&cursor->y<windowArea.y1+(I32)edgeSnapDistance){
							cursor->dragWindow.window->dock(DesktopManager::Window::DockedType::top);
						}else if(cursor->y>=windowArea.y2-(I32)edgeSnapDistance){
							cursor->dragWindow.window->dock(DesktopManager::Window::DockedType::bottom);

						}else{
							if(cursor->dragWindow.window->state!=DesktopManager::Window::State::floating){
								cursor->dragWindow.window->restore();
								cursor->regrab_window();
								// we've regrabbed from new position, so there's no move to apply yet here

							}else{
								cursor->dragWindow.window->move_to(cursor->x+cursor->dragWindow.dragOffsetX, cursor->y+cursor->dragWindow.dragOffsetY);
							}
						}

						// if(cursor->dragWindow.window->state==DesktopManager::Window::State::docked){
						// 	auto hoveredWindow = get_window_at(cursor->x, cursor->y, cursor->display);
						// 	if(hoveredWindow!=cursor->dragWindow.window){
						// 		auto other_x = hoveredWindow->get_x();
						// 		auto other_y = hoveredWindow->get_y();
						// 		auto other_width = hoveredWindow->get_width();
						// 		auto other_height = hoveredWindow->get_height();

						// 		hoveredWindow->move_and_resize_to(
						// 			cursor->dragWindow.window->get_x(), cursor->dragWindow.window->get_y(),
						// 			cursor->dragWindow.window->get_width(), cursor->dragWindow.window->get_height()
						// 		);

						// 		cursor->dragWindow.window->move_and_resize_to(
						// 			other_x, other_y,
						// 			other_width, other_height
						// 		);
						// 	}
						// }
					}

					if(!cursor->isVisible){
						cursor->isVisible = true;
						cursor->display->show();
					}
				break;
			}

			// update window under cursor
			if(window){
				switch(event.type){
					case driver::Mouse::Event::Type::moved: {
						window->_on_mouse_moved(cursor->x-window->graphicsDisplay->x, cursor->y-window->graphicsDisplay->y);
						window->events.trigger({
							type: Window::Event::Type::mouseMoved,
							mouseMoved: { event.instance, cursor->x-window->get_x(), cursor->y-window->get_y(), event.moved.x, event.moved.y }
						});
					} break;
					case driver::Mouse::Event::Type::pressed:
						window->_on_mouse_pressed(cursor->x-window->graphicsDisplay->x, cursor->y-window->graphicsDisplay->y, event.pressed.button);
						window->events.trigger({
							type: StandardWindow::Event::Type::mousePressed,
							mousePressed: { event.instance, cursor->x-window->get_x(), cursor->y-window->get_y(), event.pressed.button }
						});
					break;
					case driver::Mouse::Event::Type::released:
						window->_on_mouse_released(cursor->x-window->graphicsDisplay->x, cursor->y-window->graphicsDisplay->y, event.released.button);
						window->events.trigger({
							type: StandardWindow::Event::Type::mouseReleased,
							mouseReleased: { event.instance, cursor->x-window->get_x(), cursor->y-window->get_y(), event.released.button }
						});
					break;
					case driver::Mouse::Event::Type::scrolled: {
						window->events.trigger({
							type: Window::Event::Type::mouseScrolled,
							mouseScrolled: { event.instance, cursor->x-window->get_x(), cursor->y-window->get_y(), event.scrolled.distance }
						});
					} break;
				}
			}
		}
	}

	auto DesktopManager::_on_start() -> Try<> {
		displayManager = drivers::find_and_activate<DisplayManager>(this);
		if(!displayManager) return {"Display manager not available"};

		drivers::events.subscribe(_on_drivers_event);

		driver::Keyboard::allEvents.subscribe(on_keyboard_event);
		driver::Mouse::allEvents.subscribe(on_mouse_event);

		for(auto &mouse:drivers::iterate<Mouse>()){
			if(mouse.api.is_active()){
				_add_mouse(mouse);

			}else if(mouse.api.is_enabled()){
				TRY_IGNORE(drivers::start_driver(mouse));
			}
		}

		for(auto &keyboard:drivers::iterate<Keyboard>()){
			if(!keyboard.api.is_active()&&keyboard.api.is_enabled()){
				TRY_IGNORE(drivers::start_driver(keyboard));
			}
		}

		_update_window_area();

		return {};
	}

	auto DesktopManager::_on_stop() -> Try<> {
		// remove all cursors and mouse listeners
		while(cursors.length>0) _remove_mouse(*cursors[0].mouse);

		driver::Mouse::allEvents.unsubscribe(on_mouse_event);
		driver::Keyboard::allEvents.unsubscribe(on_keyboard_event);

		return {};
	}

	void StandardWindow::set_status(const char *set) {
		status = set;
		redraw_decorations(); //TODO: only update the text itself, not the full frame?
	}

	void StandardWindow::resize_to(U32 width, U32 height) {
		auto oldWidth = get_width();
		auto oldHeight = get_height();
		graphicsDisplay->resize_to(leftMargin+width+rightMargin, topMargin+height+bottomMargin, DeferWindowMove::depth<1);

		if(get_width()==oldWidth&&get_height()==oldHeight) return;

		if(enableTransparency){
			graphicsDisplay->buffer.draw_rect((graphics2d::Rect){0,0,(I32)graphicsDisplay->buffer.width,(I32)graphicsDisplay->buffer.height}, 0xff000000);
		}

		_draw_shadow();
		if(DeferWindowMove::depth<1){
			redraw_decorations();
		}else{
			_draw_decorations();
		}

		clientArea = graphicsDisplay->buffer.cropped(leftMargin+1, topMargin+titlebarHeight, rightMargin+1, bottomMargin+23);
		clientArea.draw_rect(0, 0, clientArea.width, clientArea.height, windowBackgroundColour);
		gui.buffer = graphicsDisplay->buffer;

		_set_titlebar_area({(I32)leftMargin, (I32)topMargin, (I32)leftMargin+(I32)width, (I32)topMargin+titlebarHeight});

		if(DeferWindowMove::depth<1){
			events.trigger({
				type: DesktopManager::Window::Event::Type::clientAreaChanged
			});

			redraw();
		}
	}
	
	void StandardWindow::redraw() {
		graphicsDisplay->update_area(client_to_graphics_area(*this, {0, 0, (I32)clientArea.width, (I32)clientArea.height}));
	}

	void StandardWindow::redraw_area(graphics2d::Rect rect) {
		graphicsDisplay->update_area(client_to_graphics_area(*this, rect));
	}

	auto DesktopManager::create_standard_window(const char *title, I32 width, I32 height) -> StandardWindow& {
		auto &window = *new driver::StandardWindow(title, width, height);
		window.move_to(
			((I32)displayManager->get_width()-width)/2,
			max(0, ((I32)displayManager->get_height()-height)/2)
		);
		window.isAutomaticPlacement = true; // cos move_to disables this

		window.draw_frame();

		windows.push_back(window);
		focusedWindow = &window;

		events.trigger({
			type: Event::Type::windowAdded,
			windowAdded: { window: &window }
		});

		update_focused_window();

		return window;
	}

	// TODO: mutex these functions

	auto DesktopManager::create_custom_window(const char *title, I32 width, I32 height) -> CustomWindow& {
		auto &window = *new driver::CustomWindow(title, width, height);
		window.move_to(
			((I32)displayManager->get_width()-width)/2,
			max(0, ((I32)displayManager->get_height()-height)/2)
		);
		window.isAutomaticPlacement = true; // cos move_to disables this

		windows.push_back(window);
		focusedWindow = &window;
		update_focused_window();

		events.trigger({
			type: Event::Type::windowAdded,
			windowAdded: { window: &window }
		});

		return window;
	}

	auto DesktopManager::get_window_from_display(DisplayManager::Display &display) -> Window* {
		for(auto window=windows.head; window; window=window->next) {
			if(window->graphicsDisplay==&display) return window;
		}

		return nullptr;
	}

	auto DesktopManager::get_total_area() -> graphics2d::Rect {
		return {0, 0, (I32)displayManager->get_width(), (I32)displayManager->get_height()};
	}

	auto DesktopManager::get_window_area() -> graphics2d::Rect {
		return windowArea;
	}

	auto DesktopManager::get_default_window_colour() -> U32 {
		return windowBackgroundColour;
	}

	auto DesktopManager::get_default_window_border_colour() -> U32 {
		return windowBorderColour;
	}

	auto DesktopManager::get_window_count() -> U32 {
		return windows.length();
	}

	// FIXME: add some extra protection around this, so that a thread doesn't try accessing a window pointer while another frees it
	auto DesktopManager::get_window(U32 index) -> Window* {
		driver::Window *window;
		for(window=windows.head; index>0; window=window->next){
			index--;
		}
		return window;
	}

	auto DesktopManager::get_focused_window() -> Window* {
		return focusedWindow;
	}
}
