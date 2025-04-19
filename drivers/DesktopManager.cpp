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
		extern graphics2d::Buffer size_x;
		extern graphics2d::Buffer size_y;
		extern graphics2d::Buffer size_nesw;
		extern graphics2d::Buffer size_nwse;
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

		struct Window: LListItem<Window>, virtual DesktopManager::Window {
			U64 timeCreated = time::now();

			static const auto resizeMargin = 4;

			enum struct CursorArea {
				none,
				body,
				titlebar,
				sizeN,
				sizeNW,
				sizeNE,
				sizeS,
				sizeSW,
				sizeSE,
				sizeW,
				sizeE
			};

			UVec2 minSize{32,32};
			UVec2 maxSize{(U32)~0, (U32)~0};

			struct Gui: ui2d::Gui {
				typedef ui2d::Gui Super;

				/**/ Gui():
					Super(graphics2d::Buffer(), driver::theme)
				{}

				void update_area(graphics2d::Rect rect) {
					if(isFrozen) return;
					window().graphicsDisplay->update_area(rect);
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

			auto get_width() -> U32 override {
				return graphicsDisplay->get_width()-leftMargin-rightMargin;
			}

			auto get_height() -> U32 override {
				return graphicsDisplay->get_height()-topMargin-bottomMargin;
			}

			auto get_window_area() -> graphics2d::Buffer& override {
				return graphicsDisplay->buffer;
			}

			auto get_client_area() -> graphics2d::Buffer& override {
				return clientArea;
			}

			virtual auto get_cursor_area_at(I32 x, I32 y) -> CursorArea {
				// in case the window is tiny, so we can cut off at halfway
				const auto isTop = y<=(I32)get_height()/2;
				const auto isLeft = x<=(I32)get_width()/2;

				const auto cornerWrap = 4;

				if(isTop&&maths::abs(y)<(I32)resizeMargin&&(state==State::floating||state==State::docked&&dockedType==DockedType::bottom)){
					if(state!=State::docked&&isLeft&&maths::abs(x)<(I32)resizeMargin*cornerWrap){
						return CursorArea::sizeNW;
					}else if(state!=State::docked&&maths::abs(x-(I32)get_width())<(I32)resizeMargin*cornerWrap){
						return CursorArea::sizeNE;
					}else{
						return CursorArea::sizeN;
					}

				}else if(maths::abs(y-(I32)get_height())<(I32)resizeMargin&&(state==State::floating||state==State::docked&&dockedType==DockedType::top)){
					if(state!=State::docked&&isLeft&&maths::abs(x)<(I32)resizeMargin*cornerWrap){
						return CursorArea::sizeSW;
					}else if(state!=State::docked&&maths::abs(x-(I32)get_width())<(I32)resizeMargin*cornerWrap){
						return CursorArea::sizeSE;
					}else{
						return CursorArea::sizeS;
					}

				}else if(isLeft&&maths::abs(x)<(I32)resizeMargin&&(state==State::floating||state==State::docked&&dockedType==DockedType::right)){
					if(state!=State::docked&&isTop&&maths::abs(y)<(I32)resizeMargin*cornerWrap){
						return CursorArea::sizeNW;
					}else if(state!=State::docked&&maths::abs(y-(I32)get_height())<(I32)resizeMargin*cornerWrap){
						return CursorArea::sizeSW;
					}else{
						return CursorArea::sizeW;
					}

				}else if(maths::abs(x-(I32)get_width())<(I32)resizeMargin&&(state==State::floating||state==State::docked&&dockedType==DockedType::left)){
					if(state!=State::docked&&isTop&&maths::abs(y)<(I32)resizeMargin*cornerWrap){
						return CursorArea::sizeNE;
					}else if(state!=State::docked&&maths::abs(y-(I32)get_height())<(I32)resizeMargin*cornerWrap){
						return CursorArea::sizeSE;
					}else{
						return CursorArea::sizeE;
					}
				}

				if(x<0||y<0||x>=(I32)get_width()||y>=(I32)get_height()) return CursorArea::none;

				if(titlebarArea.contains(leftMargin+x, topMargin+y)) return CursorArea::titlebar;
				
				return CursorArea::body;
			}

			auto get_top_margin() -> U32 override;
			auto get_bottom_margin() -> U32 override;
			auto get_left_margin() -> U32 override;
			auto get_right_margin() -> U32 override;

			void move_to(I32 x, I32 y) override;
			void resize_to(U32 width, U32 height) override;
			void move_and_resize_to(I32 x, I32 y, U32 width, U32 height) override;

			virtual void _on_graphicsDisplay_resized();

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

			void set_size_limits(U32 minWidth, U32 minHeight, U32 maxWidth, U32 maxHeight) override {
				if(minSize.x==minWidth&&minSize.y==minHeight&&maxSize.x==maxWidth&&maxSize.y==maxHeight) return;

				minSize.x = minWidth;
				minSize.y = minHeight;
				maxSize.x = std::max(minWidth, maxWidth);
				maxSize.y = std::max(minHeight, maxHeight);

				const auto currentWidth = (U32)get_width();
				const auto currentHeight = (U32)get_height();

				const auto newWidth = maths::clamp(currentWidth, minSize.x, maxSize.x);
				const auto newHeight = maths::clamp(currentHeight, minSize.y, maxSize.y);

				if(newWidth!=currentWidth||newHeight!=currentHeight){
					resize_to(newWidth, newHeight);
				}
			}

			auto is_top() -> bool {
				return graphicsDisplay->is_top();
			}

			auto is_visible() -> bool {
				return graphicsDisplay->isVisible;
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

				graphicsDisplay->update_area(graphics2d::Rect{0, 0, (I32)leftMargin+(I32)get_width()+(I32)rightMargin, (I32)topMargin});
				graphicsDisplay->update_area(graphics2d::Rect{0, (I32)topMargin, (I32)leftMargin, (I32)topMargin+(I32)get_height()});
				graphicsDisplay->update_area(graphics2d::Rect{(I32)leftMargin+(I32)get_width(), (I32)topMargin, (I32)leftMargin+(I32)get_width()+(I32)rightMargin, (I32)topMargin+(I32)get_height()});
				graphicsDisplay->update_area(graphics2d::Rect{0, (I32)topMargin+(I32)get_height(), (I32)leftMargin+(I32)get_width()+(I32)rightMargin, (I32)topMargin+(I32)get_height()+(I32)bottomMargin});
			}

			void redraw() {
				graphicsDisplay->update_area({(I32)leftMargin, (I32)topMargin, (I32)leftMargin+(I32)get_width(), (I32)topMargin+(I32)get_height()});
			}

			void redraw_area(graphics2d::Rect rect) {
				graphicsDisplay->update_area(rect.offset(leftMargin, topMargin));
			}

			virtual void _set_titlebar_area(graphics2d::Rect rect) {
				titlebarArea = rect;
			}

			virtual void _on_mouse_left() {}
			virtual void _on_mouse_moved(I32 x, I32 y) {}
			virtual void _on_mouse_pressed(I32 x, I32 y, U32 button) {}
			virtual void _on_mouse_released(I32 x, I32 y, U32 button) {}

			DisplayManager::Display *graphicsDisplay;
			const char *title;
			graphics2d::Buffer clientArea;
			graphics2d::Rect titlebarArea;

			Cursor *draggingCursor = nullptr;
			State state = State::floating;

			bool isAutomaticPlacement = true;

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
		void _autoposition_window(Window&);
		void _autoreposition_windows();
		void _update_window_area(Window *exclude = nullptr);

		auto Window::get_top_margin() -> U32 {
			return topMargin;
		}
		auto Window::get_bottom_margin() -> U32 {
			return bottomMargin;
		}
		auto Window::get_left_margin() -> U32 {
			return leftMargin;
		}
		auto Window::get_right_margin() -> U32 {
			return rightMargin;
		}

		void Window::move_to(I32 x, I32 y) {
			isAutomaticPlacement = false;

			if(x==get_x()&&y==get_y()) return;

			const auto margin = 10;
			x = maths::clamp(x, -(I32)graphicsDisplay->buffer.width+margin, (I32)displayManager->get_width()-margin);
			y = maths::clamp(y, 0, (I32)displayManager->get_height()-margin);

			graphicsDisplay->move_to(x-(I32)leftMargin, y-(I32)topMargin);
		}

		void Window::_on_graphicsDisplay_resized() {
			if(enableTransparency){
				graphicsDisplay->buffer.draw_rect((graphics2d::Rect){0,0,(I32)graphicsDisplay->buffer.width,(I32)graphicsDisplay->buffer.height}, 0xff000000);
			}

			_update_window_area();

			_draw_decorations();

			events.trigger({
				type: DesktopManager::Window::Event::Type::clientAreaChanged
			});

			graphicsDisplay->update();
		}

		void Window::resize_to(U32 width, U32 height) {
			width = maths::clamp(width, minSize.x, maxSize.x);
			height = maths::clamp(height, minSize.y, maxSize.y);

			auto oldWidth = get_width();
			auto oldHeight = get_height();
			graphicsDisplay->resize_to(leftMargin+width+rightMargin, topMargin+height+bottomMargin);
			clientArea = graphicsDisplay->buffer.cropped(leftMargin, topMargin, rightMargin, bottomMargin);

			auto deltaX = (I32)get_width()-(I32)oldWidth;
			auto deltaY = (I32)get_height()-(I32)oldHeight;

			if(deltaX==0&&deltaY==0) return;

			graphicsDisplay->solidArea.x2 += max(-(I32)graphicsDisplay->solidArea.width(), deltaX);
			graphicsDisplay->solidArea.y2 += max(-(I32)graphicsDisplay->solidArea.height(), deltaY);
			graphicsDisplay->interactArea.x2 += max(-(I32)graphicsDisplay->interactArea.width(), deltaX);
			graphicsDisplay->interactArea.y2 += max(-(I32)graphicsDisplay->interactArea.height(), deltaY);
			titlebarArea.x2 += max(-(I32)titlebarArea.width(), deltaX);
			titlebarArea.y2 += max(-(I32)titlebarArea.height(), deltaY);

			_on_graphicsDisplay_resized();
		}

		void Window::move_and_resize_to(I32 x, I32 y, U32 width, U32 height) {
			isAutomaticPlacement = false;

			width = maths::clamp(width, minSize.x, maxSize.x);
			height = maths::clamp(height, minSize.y, maxSize.y);

			auto oldWidth = get_width();
			auto oldHeight = get_height();

			if(width==oldWidth&&height==oldHeight) return move_to(x, y);

			graphicsDisplay->move_and_resize_to({x-(I32)leftMargin, y-(I32)topMargin, x+(I32)width+(I32)rightMargin, y+(I32)height+(I32)bottomMargin});

			auto deltaX = (I32)get_width()-(I32)oldWidth;
			auto deltaY = (I32)get_height()-(I32)oldHeight;

			clientArea = graphicsDisplay->buffer.cropped(leftMargin, topMargin, rightMargin, bottomMargin);

			graphicsDisplay->solidArea.x2 += max(-(I32)graphicsDisplay->solidArea.width(), deltaX);
			graphicsDisplay->solidArea.y2 += max(-(I32)graphicsDisplay->solidArea.height(), deltaY);
			graphicsDisplay->interactArea.x2 += max(-(I32)graphicsDisplay->interactArea.width(), deltaX);
			graphicsDisplay->interactArea.y2 += max(-(I32)graphicsDisplay->interactArea.height(), deltaY);
			titlebarArea.x2 += max(-(I32)titlebarArea.width(), deltaX);
			titlebarArea.y2 += max(-(I32)titlebarArea.height(), deltaY);

			_on_graphicsDisplay_resized();
		}

		void Window::show() {
			if(graphicsDisplay->isVisible) return;

			if(isAutomaticPlacement){
				_autoposition_window(*this);
			}

			graphicsDisplay->show();
			if(state==State::docked){
				_enable_docked_window(*this);
			}

			DesktopManager::instance.events.trigger({
				type: DesktopManager::Event::Type::windowShown,
				windowShown: { window: this }
			});	

			// _autoreposition_windows();
		}
		
		void Window::hide() {
			if(!graphicsDisplay->isVisible) return;

			graphicsDisplay->hide();
			if(state==State::docked){
				_disable_docked_window(*this);
			}

			DesktopManager::instance.events.trigger({
				type: DesktopManager::Event::Type::windowHidden,
				windowHidden: { window: this }
			});	
		}

		void Window::dock(DockedType type) {
			switch(state){
				case State::docked:
					if(dockedType==type) return;

					_disable_docked_window(*this);
				break;
				case State::floating:
					defaultFloatingRect = {get_x(), get_y(), get_x()+(I32)get_width(), get_y()+(I32)get_height()};
				break;
				case State::minimised:
					graphicsDisplay->show();
					defaultFloatingRect = {get_x(), get_y(), get_x()+(I32)get_width(), get_y()+(I32)get_height()};
				break;
			}

			state = State::docked;
			dockedType = type;

			auto windowArea = DesktopManager::instance.get_window_area();

			switch(dockedType){
				case DockedType::top:
					maxSize.x = (U32)~0;
					move_and_resize_to(
						windowArea.x1, windowArea.y1,
						windowArea.width(), max(16, min(windowArea.height()/2-4, (I32)maxDockedHeight))
					);
				break;
				case DockedType::bottom: {
					maxSize.x = (U32)~0;
					const auto height = max(16, min(windowArea.height()/2-4, (I32)maxDockedHeight));
					move_and_resize_to(
						windowArea.x1, windowArea.y2-height,
						windowArea.width(), height
					);
				} break;
				case DockedType::left:
					maxSize.y = (U32)~0;
					move_and_resize_to(
						windowArea.x1, windowArea.y1,
						max(16, min(windowArea.width()/2-4, (I32)maxDockedWidth)), windowArea.height()
					);
				break;
				case DockedType::right: {
					maxSize.y = (U32)~0;
					const auto width = max(16, min(windowArea.width()/2-4, (I32)maxDockedWidth));
					move_and_resize_to(
						windowArea.x2-width, windowArea.y1,
						width, windowArea.height()
					);
				} break;
				case DockedType::full:
					maxSize.x = (U32)~0;
					maxSize.y = (U32)~0;
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

			U32 titlebarAreaIndentLeft = 0;
			U32 titlebarAreaIndentRight = 0;

			static const auto widgetSpacing = 4;

			static const auto cornerRadius = enableTransparency?5:2;
			U32 corner[cornerRadius+1];
			U32 cornerInner[cornerRadius-1+1];

			const char *status = "";

			/**/ StandardWindow(const char *title, U32 width, U32 height):
				Super(title, maths::max(theme.get_window_min_width(),width)+theme.get_window_left_margin()+theme.get_window_right_margin(), maths::max(theme.get_window_min_height(),height)+theme.get_window_top_margin()+theme.get_window_bottom_margin()),
				closeButton(gui, {0,0,0,0}, 0xff0000, ""),
				maximiseButton(gui, {0,0,0,0}, 0xff8800, "")
			{
				leftMargin = theme.get_window_left_margin();
				topMargin = theme.get_window_top_margin();
				rightMargin = theme.get_window_right_margin();
				bottomMargin = theme.get_window_bottom_margin();

				closeButton.icon = &ui2d::image::widgets::close;
				maximiseButton.icon = &ui2d::image::widgets::maximise;

				minSize.x = theme.get_window_min_width();
				minSize.y = theme.get_window_min_height();

				width = maths::max(minSize.x, width);
				height = maths::max(minSize.y, height);

				if(graphicsDisplay){
					clientArea = graphicsDisplay->buffer.cropped(leftMargin, topMargin, rightMargin, bottomMargin);
					gui.buffer = graphicsDisplay->buffer;
					if(enableTransparency){
						graphicsDisplay->buffer.draw_rect((graphics2d::Rect){0,0,(I32)graphicsDisplay->buffer.width,(I32)graphicsDisplay->buffer.height}, 0xff000000);
					}
				}

				_set_titlebar_area(theme.get_window_titlebar_area({0, 0, (I32)graphicsDisplay->buffer.width, (I32)graphicsDisplay->buffer.height}));

				gui.redraw(false);

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
				return graphics2d::Rect{(I32)leftMargin, (I32)topMargin, (I32)leftMargin+(I32)get_width(), (I32)topMargin+(I32)get_height()};
			}

			auto get_cursor_area_at(I32 x, I32 y) -> CursorArea override {
				return Super::get_cursor_area_at(x, y);
			}

			void set_status(const char*) override;

			void resize_to(U32 width, U32 height) override;
			void move_and_resize_to(I32 x, I32 y, U32 width, U32 height) override;
			void _on_graphicsDisplay_resized() override;

			void redraw() override;
			void redraw_area(graphics2d::Rect) override;

			void set_size_limits(U32 minWidth, U32 minHeight, U32 maxWidth, U32 maxHeight) override {
				Super::set_size_limits(maths::max(theme.get_window_min_width(), minWidth), maths::max(theme.get_window_min_height(), minHeight), maxWidth, maxHeight);
			}

			void _set_titlebar_area(graphics2d::Rect rect) override {
				Super::_set_titlebar_area(rect);

				const auto size = titlebarArea.height()-widgetSpacing*2;

				closeButton.rect = {titlebarArea.x2-widgetSpacing-size, titlebarArea.y1+widgetSpacing, titlebarArea.x2-widgetSpacing, titlebarArea.y2-widgetSpacing};
				maximiseButton.rect = {closeButton.rect.x1-widgetSpacing-1-widgetSpacing-size, titlebarArea.y1+widgetSpacing, closeButton.rect.x1-widgetSpacing-1-widgetSpacing, titlebarArea.y2-widgetSpacing};
				minimiseButton.rect = {maximiseButton.rect.x1-widgetSpacing-size, titlebarArea.y1+widgetSpacing, maximiseButton.rect.x1-widgetSpacing, titlebarArea.y2-widgetSpacing};

				titlebarAreaIndentLeft = maths::max(0, widgetSpacing-1);
				titlebarAreaIndentRight = titlebarArea.x2-minimiseButton.rect.x1+maths::max(0, widgetSpacing-1);
			}

			void _draw_decorations() override {
				theme.draw_window_frame(graphicsDisplay->buffer, {0, 0, (I32)graphicsDisplay->buffer.width, (I32)graphicsDisplay->buffer.height}, {
					.isFocused = _draw_focused,
					.title = title,
					.status = status,
					.titlebarAreaIndentLeft = titlebarAreaIndentLeft,
					.titlebarAreaIndentRight = titlebarAreaIndentRight,
				});

				closeButton.opacity = _draw_focused?0xff:0x66;
				maximiseButton.opacity = _draw_focused?0xff:0x66;
				minimiseButton.opacity = _draw_focused?0xff:0x66;

				{ // draw widget divide
					graphicsDisplay->buffer.draw_line(closeButton.rect.x1-widgetSpacing-1, titlebarArea.y1+widgetSpacing+1, closeButton.rect.x1-widgetSpacing-1, titlebarArea.y2-widgetSpacing-1, theme.get_window_titlebar_divider_colour({.isFocused = _draw_focused}));
				}

				gui.redraw(false);
			}

			void redraw_decorations() override {
				_draw_decorations();

				// we assume the border is the gap between the interaction area (the main window body) and the inner client area
				const auto interactArea = theme.get_window_interact_area({0, 0, (I32)graphicsDisplay->buffer.width, (I32)graphicsDisplay->buffer.height});
				const auto clientArea = theme.get_window_client_area({0, 0, (I32)graphicsDisplay->buffer.width, (I32)graphicsDisplay->buffer.height});

				//top
				graphicsDisplay->update_area(graphics2d::Rect{interactArea.x1, interactArea.y1, interactArea.x2, clientArea.y1});
				//left
				graphicsDisplay->update_area(graphics2d::Rect{interactArea.x1, clientArea.y1, clientArea.x1, clientArea.y2});
				//right
				graphicsDisplay->update_area(graphics2d::Rect{clientArea.x2, clientArea.y1, interactArea.x2, clientArea.y2});
				//bottom
				graphicsDisplay->update_area(graphics2d::Rect{interactArea.x1, clientArea.x2, interactArea.x2, interactArea.y2});
			}

			void _draw_frame() {
				// auto rect = get_border_rect();

				// U32 corner[4] = {3, 1, 1, (U32)~0};
				// U32 cornerInner[3] = {2, 1, (U32)~0};

				auto &display = *graphicsDisplay;

				_draw_decorations();
				{
					for(auto i=0u;i<sizeof(graphicsDisplay->topLeftCorner)/sizeof(graphicsDisplay->topLeftCorner[0]);i++){
						graphicsDisplay->topLeftCorner[i] = 0;
						graphicsDisplay->topRightCorner[i] = 0;
						graphicsDisplay->bottomLeftCorner[i] = 0;
						graphicsDisplay->bottomRightCorner[i] = 0;
					}
					graphicsDisplay->solidArea = theme.get_window_solid_area({0, 0, (I32)graphicsDisplay->buffer.width, (I32)graphicsDisplay->buffer.height});
					graphicsDisplay->interactArea = theme.get_window_interact_area({0, 0, (I32)graphicsDisplay->buffer.width, (I32)graphicsDisplay->buffer.height});	
				}

				const auto clientRect = theme.get_window_client_area({0, 0, (I32)graphicsDisplay->buffer.width, (I32)graphicsDisplay->buffer.height});
				clientArea = display.buffer.region(clientRect.x1, clientRect.y1, clientRect.width(), clientRect.height());
				clientArea.draw_rect(0, 0, clientArea.width, clientArea.height, windowBackgroundColour);
				gui.redraw(false);
			}

			void redraw_shadow() {
				auto &buffer = graphicsDisplay->buffer;

				const auto leftShadow = theme.get_window_left_margin();
				const auto rightShadow = theme.get_window_right_margin();
				const auto topShadow = theme.get_window_top_margin();
				const auto bottomShadow = theme.get_window_bottom_margin();

				graphicsDisplay->update_area({(I32)leftShadow, 0, (I32)buffer.width-(I32)rightShadow, (I32)topShadow});
				graphicsDisplay->update_area({0, 0, (I32)leftShadow, (I32)buffer.height});
				graphicsDisplay->update_area({(I32)leftShadow+(I32)get_width(), 0, (I32)leftShadow+(I32)get_width()+(I32)rightShadow, (I32)buffer.height});
				graphicsDisplay->update_area({(I32)leftShadow, (I32)topShadow+(I32)get_height(), (I32)get_width(), (I32)topShadow+(I32)get_height()+(I32)bottomShadow});
			}

			void _on_mouse_left() override {
				gui.on_mouse_left();
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
				_set_titlebar_area(set);
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

			void set_corner(U32 *topLeft, U32 *topRight, U32 *bottomLeft, U32 *bottomRight) override {
				U32 topLeftRadius = 0; if(topLeft) while(topLeftRadius[topLeft]!=~0u) topLeftRadius++;
				U32 topRightRadius = 0; if(topRight) while(topRightRadius[topRight]!=~0u) topRightRadius++;
				U32 bottomLeftRadius = 0; if(bottomLeft) while(bottomLeftRadius[bottomLeft]!=~0u) bottomLeftRadius++;
				U32 bottomRightRadius = 0; if(bottomRight) while(bottomRightRadius[bottomRight]!=~0u) bottomRightRadius++;

				memcpy(graphicsDisplay->topLeftCorner, topLeft, topLeftRadius*sizeof(U32));
				memcpy(graphicsDisplay->topRightCorner, topRight, topRightRadius*sizeof(U32));
				memcpy(graphicsDisplay->bottomLeftCorner, bottomLeft, bottomLeftRadius*sizeof(U32));
				memcpy(graphicsDisplay->bottomRightCorner, bottomRight, bottomRightRadius*sizeof(U32));
			}
		};

		auto get_window_at(I32 x, I32 y, bool includeMargin = false, DisplayManager::Display *below = nullptr) -> Window* {
			auto display = displayManager->get_display_at(x, y, false, below, includeMargin?Window::resizeMargin:0);
			auto windowInterface = display?DesktopManager::instance.get_window_from_display(*display):nullptr;
			auto window = windowInterface?(Window*)(StandardWindow*)windowInterface->as_standardWindow()?:(Window*)(CustomWindow*)windowInterface->as_customWindow():nullptr;

			return window;
		}

		struct Cursor {
			/**/ Cursor(){}

			/**/ Cursor(Mouse &mouse, I32 x, I32 y):
				mouse(&mouse),
				display(displayManager->create_display(nullptr, DisplayManager::DisplayLayer::cursor, ui2d::image::cursors::_default.width, ui2d::image::cursors::_default.height)),
				x(x),
				y(y)
			{
				display->move_to(x, y);
				display->solidArea.clear();
				set_cursor(&ui2d::image::cursors::_default, 0, 0);
			}

			Mouse *mouse = nullptr;
			DisplayManager::Display *display;
			graphics2d::Buffer *currentCursor = nullptr;
			Window *lastHoveredWindow = nullptr;
			I32 x = 0, y = 0;
			I32 offset[2] = {0,0};
			bool isVisible = false;

			struct Vec {
				I32 x;
				I32 y;
			};

			struct GrabbedWindow {
				enum struct GrabType {
					move,
					size_n,
					size_nw,
					size_ne,
					size_s,
					size_sw,
					size_se,
					size_w,
					size_e
				};

				GrabType grabType;
				Window *window = nullptr;
				I32 grabOffsetX;
				I32 grabOffsetY;

				I32 startX;
				I32 startY;
				bool active; // on move, set to false until the mouse moves activationDistance from startX/Y
				static const auto activationDistance = 10;
			} grabbedWindow;

			void grab_window(Window &window, GrabbedWindow::GrabType grabType) {
				if(window.draggingCursor&&window.draggingCursor!=this){
					window.draggingCursor->release_window();
				}

				grabbedWindow.startX = x;
				grabbedWindow.startY = y;
				switch(grabType){
					case GrabbedWindow::GrabType::move:
						// moves require an activation distance to begin the drag
						grabbedWindow.active = false;
					break;
					case GrabbedWindow::GrabType::size_n:
					case GrabbedWindow::GrabType::size_nw:
					case GrabbedWindow::GrabType::size_ne:
					case GrabbedWindow::GrabType::size_s:
					case GrabbedWindow::GrabType::size_sw:
					case GrabbedWindow::GrabType::size_se:
					case GrabbedWindow::GrabType::size_w:
					case GrabbedWindow::GrabType::size_e:
						// edge sizings begin drag immediately
						grabbedWindow.active = true;
					break;
				}
				grabbedWindow.grabType = grabType;
				grabbedWindow.window = &window;

				switch(grabType){
					case GrabbedWindow::GrabType::size_n:
					case GrabbedWindow::GrabType::size_nw:
					case GrabbedWindow::GrabType::size_ne:
					case GrabbedWindow::GrabType::size_s:
					case GrabbedWindow::GrabType::size_sw:
					case GrabbedWindow::GrabType::size_se:
					case GrabbedWindow::GrabType::size_w:
					case GrabbedWindow::GrabType::size_e: {
						const auto isLeft = grabType==Cursor::GrabbedWindow::GrabType::size_nw||grabType==Cursor::GrabbedWindow::GrabType::size_w||grabType==Cursor::GrabbedWindow::GrabType::size_sw;
						const auto isRight = grabType==Cursor::GrabbedWindow::GrabType::size_ne||grabType==Cursor::GrabbedWindow::GrabType::size_e||grabType==Cursor::GrabbedWindow::GrabType::size_se;
						const auto isTop = grabType==Cursor::GrabbedWindow::GrabType::size_nw||grabType==Cursor::GrabbedWindow::GrabType::size_n||grabType==Cursor::GrabbedWindow::GrabType::size_ne;
						const auto isBottom = grabType==Cursor::GrabbedWindow::GrabType::size_sw||grabType==Cursor::GrabbedWindow::GrabType::size_s||grabType==Cursor::GrabbedWindow::GrabType::size_se;

						grabbedWindow.grabOffsetX = window.get_width()-x*(isLeft?-1:isRight?+1:0);
						grabbedWindow.grabOffsetY = window.get_height()-y*(isTop?-1:isBottom?+1:0);
					} break;
					case GrabbedWindow::GrabType::move:
						grabbedWindow.grabOffsetX = window.get_x()-x;
						grabbedWindow.grabOffsetY = window.get_y()-y;
					break;
				}

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

			void show() {
				if(isVisible) return;
				isVisible = true;
				display->show();
			}

			void hide() {
				if(!isVisible) return;
				isVisible = false;
				display->hide();
			}

			void release_window() {
				if(!grabbedWindow.window||grabbedWindow.window->draggingCursor!=this) return;

				grabbedWindow.window->draggingCursor = nullptr;
				grabbedWindow.window->redraw_decorations();
				grabbedWindow.window = nullptr;
			}

			// only valid if the window was already in a move grab
			void regrab_window_move() {
				grabbedWindow.startX = x;
				grabbedWindow.startY = y;
				grabbedWindow.grabOffsetX = maths::clamp(grabbedWindow.grabOffsetX, -(I32)grabbedWindow.window->titlebarArea.x2+8, -(I32)grabbedWindow.window->titlebarArea.x1-8);
				grabbedWindow.grabOffsetY = maths::clamp(grabbedWindow.grabOffsetY, -(I32)grabbedWindow.window->titlebarArea.y2+8, -(I32)grabbedWindow.window->titlebarArea.y1-8);
			}

			void set_cursor(graphics2d::Buffer *newCursor, I32 originX, I32 originY){
				if(currentCursor==newCursor) return;

				currentCursor = newCursor;

				if(currentCursor){
					if(display->buffer.width!=currentCursor->width||display->buffer.height!=currentCursor->height){
						if(isVisible){
							display->hide();
						}
						display->resize_to(currentCursor->width, currentCursor->height);
						display->buffer.draw_buffer(0, 0, 0, 0, currentCursor->width, currentCursor->height, *currentCursor);
						if(isVisible){
							display->show(false);
						}
					}else{
						display->buffer.draw_buffer(0, 0, 0, 0, currentCursor->width, currentCursor->height, *currentCursor);
					}
				}else{
					display->buffer.draw_rect(graphics2d::Rect{0, 0, (I32)display->buffer.width, (I32)display->buffer.height}, 0xff000000);
				}

				if(offset[0]!=originX||offset[1]!=originY){
					offset[0] = originX;
					offset[1] = originY;
					set_position(x, y);
				}else{
					display->update();
				}
			}

			void set_position(U32 x, U32 y){
				this->x = x;
				this->y = y;
				display->move_to(x-offset[0], y-offset[1]);
			}
		};

		LList<Window> windows;
		Window *focusedWindow = nullptr;
		ListUnordered<Cursor> cursors; // do not preallocate any (dynamic allocations on boot fail before memory is setup)

		void _add_mouse(Mouse &mouse);
		void _remove_mouse(Mouse &mouse);
		auto _find_cursor(Mouse &mouse) -> Cursor*;

		void _on_drivers_event(const drivers::Event &event) {
			switch(event.type){
				case drivers::Event::Type::driverInstalled:
					// turn on any new mouse drivers as they come in, so we can capture them on start
					if(auto mouse = event.driverInstalled.driver->as_type<Mouse>()){
						if(mouse->api.is_enabled()){
							TRY_IGNORE(drivers::start_driver(*mouse));
						}
					}
				break;
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
			cursors.push({mouse, x, y});
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

		void _autoposition_window(Window &window) {
			// find a previous window this can lean against
			auto previous = window.prev;
			for(; previous; previous=previous->prev){
				// skip non-floating windows such as minimised and docked
				if(previous->state!=Window::State::floating) continue;

				// if we hit a tall window, then abort as there's nothing decent to rest against
				if(previous->get_height()>=windowArea.height()*8u/10u){
					previous = nullptr;
					break;
				}

				// take this candidate
				break;
			}

			auto x = max(8, (I32)windowArea.width()/2-(I32)window.get_width()/2);
			auto y = windowArea.height()*1/5;

			if(previous){
				const auto newY = previous->get_y()-windowArea.y1+theme.get_window_titlebar_area({0,0,(I32)previous->graphicsDisplay->buffer.width,(I32)previous->graphicsDisplay->buffer.height}).y2;
				if(newY<windowArea.height()*3/4){
					y = newY;
				}
			}

			window.move_to(windowArea.x1+x, windowArea.y1+y);
			window.isAutomaticPlacement = true;
		}

		void _autoreposition_windows() {
			const auto now = time::now();
			const auto repositionDuration = 1000000; // windows created within the last second

			{
				auto repositionIndex = 0u;

				for(auto window=windows.head; window; window=window->next){
					if(!window->isAutomaticPlacement) continue;
					if(window->timeCreated+repositionDuration<now) continue;

					const auto isWide = window->get_width()>=windowArea.width()*7u/10u;
					const auto isTall = window->get_height()>=windowArea.height()*7u/10u;

					if(isWide&&isTall) continue; // don't move big windows (that are both tall and wide)

					const auto margin = max(8, min(windowArea.width(), windowArea.height())/30);

					auto left = margin;
					auto right = windowArea.width()-window->get_width()-margin;
					auto centreX = (left+right)/2;

					auto top = margin;
					auto bottom = max(0, (I32)windowArea.height()-(I32)window->get_height()-margin);
					auto centreY = (top+bottom)/2;

					// // move toward centre by 20%
					// left = left*4/5 + centreX*1/5;
					// right = right*4/5 + centreX*1/5;
					// top = top*4/5 + centreY*1/5;
					// bottom = bottom*4/5 + centreY*1/5;

					if(isTall){
						window->move_to(windowArea.x1+(repositionIndex%2?right:left), centreY);

					}else if(isWide){
						window->move_to(centreX, windowArea.y1+(repositionIndex%2?bottom:top));

					}else{
						window->move_to(
							windowArea.x1+(repositionIndex%2?
								right
							:
								left
							),
							windowArea.y1+((repositionIndex/2)%2?
								bottom
							:
								top
							)
						);
					}

					window->isAutomaticPlacement = true; // in case we need to move it again

					repositionIndex++;
				}
			}
		}

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
										if(window->get_x()>=dockedWindow.get_x()+(I32)dockedWindow.get_width()){
											window->move_and_resize_to(window->get_x()-(I32)dockedWindow.get_width(), window->get_y(), (I32)window->get_width()+(I32)dockedWindow.get_width(), window->get_height());
										}
									break;
									case DesktopManager::Window::DockedType::left:
										if(window->get_x()>=dockedWindow.get_x()+(I32)dockedWindow.get_width()){
											window->move_to(window->get_x()-(I32)dockedWindow.get_width(), window->get_y());
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
										if(window->get_x()+(I32)window->get_width()<=dockedWindow.get_x()){
											window->resize_to(window->get_width()+dockedWindow.get_width(), window->get_height());
										}
									break;
									case DesktopManager::Window::DockedType::left:
									break;
									case DesktopManager::Window::DockedType::right:
										if(window->get_x()+(I32)window->get_width()<=dockedWindow.get_x()){
											window->move_to(window->get_x()+(I32)dockedWindow.get_width(), window->get_y());
										}
									break;
								}
							break;
							case DesktopManager::Window::DockedType::top:
								switch(window->dockedType){
									case DesktopManager::Window::DockedType::left:
									case DesktopManager::Window::DockedType::right:
									case DesktopManager::Window::DockedType::full:
										if(window->get_y()>=dockedWindow.get_y()+(I32)dockedWindow.get_height()){
											window->move_and_resize_to(window->get_x(), window->get_y()-(I32)dockedWindow.get_height(), window->get_width(), window->get_height()+(I32)dockedWindow.get_height());
										}
									break;
									case DesktopManager::Window::DockedType::top:
										if(window->get_y()>=dockedWindow.get_y()+(I32)dockedWindow.get_height()){
											window->move_to(window->get_x(), window->get_y()-(I32)dockedWindow.get_height());
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
										if(window->get_y()+(I32)window->get_height()<=dockedWindow.get_y()){
											window->resize_to(window->get_width(), window->get_height()+dockedWindow.get_height());
										}
									break;
									case DesktopManager::Window::DockedType::top:
									break;
									case DesktopManager::Window::DockedType::bottom:
										if(window->get_y()+(I32)window->get_height()<=dockedWindow.get_y()){
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
							windowArea = windowArea.intersect({0, window->get_y()+(I32)window->get_height(), (I32)displayManager->get_width(), (I32)displayManager->get_height()});
						break;
						case Window::DockedType::bottom:
							windowArea = windowArea.intersect({0, 0, (I32)displayManager->get_width(), window->get_y()});
						break;
						case Window::DockedType::left:
							windowArea = windowArea.intersect({window->get_x()+(I32)window->get_width(), 0, (I32)displayManager->get_width(), (I32)displayManager->get_height()});
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
						auto area = (graphics2d::Rect){window->get_x(), window->get_y(), window->get_x()+(I32)window->get_width(), window->get_y()+(I32)window->get_height()};

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

			if(event.type==Mouse::Event::Type::moved){
				cursor->set_position(
					maths::clamp(cursor->x+event.moved.x, 0, (I32)displayManager->get_width()-1),
					maths::clamp(cursor->y+event.moved.y, 0, (I32)displayManager->get_height()-1)
				);
			}

			auto window = get_window_at(cursor->x, cursor->y, false, cursor->display);
			auto marginedWindow = get_window_at(cursor->x, cursor->y, true, cursor->display);

			auto cursorWindow = marginedWindow;
			auto cursorWindowArea = cursorWindow?cursorWindow->get_cursor_area_at(cursor->x-cursorWindow->get_x(), cursor->y-cursorWindow->get_y()):Window::CursorArea::none;

			if(cursorWindowArea==Window::CursorArea::none&&window){
				cursorWindow = window;
				cursorWindowArea = cursorWindow->get_cursor_area_at(cursor->x-cursorWindow->get_x(), cursor->y-cursorWindow->get_y());
			}

			const auto cursorWasDefault = cursor->currentCursor==&ui2d::image::cursors::_default||cursor->currentCursor==&ui2d::image::cursors::_default_left||cursor->currentCursor==&ui2d::image::cursors::_default_right;

			// update cursor
			switch(event.type){
				case Mouse::Event::Type::pressed: {
					if(cursorWindow) {
						cursorWindow->raise();
						focusedWindow = cursorWindow;
						update_focused_window();
					}else{
						focusedWindow = nullptr;
						update_focused_window();
					}

					if(event.pressed.button==0){
						if(cursorWasDefault) cursor->set_cursor(&ui2d::image::cursors::_default_left, 0, 0);
						if(!cursor->grabbedWindow.window&&cursorWindow){
							auto grabType = Cursor::GrabbedWindow::GrabType::move;

							switch(cursorWindowArea){
								case Window::CursorArea::sizeN:
									grabType = Cursor::GrabbedWindow::GrabType::size_n;
								break;
								case Window::CursorArea::sizeNW:
									grabType = Cursor::GrabbedWindow::GrabType::size_nw;
								break;
								case Window::CursorArea::sizeNE:
									grabType = Cursor::GrabbedWindow::GrabType::size_ne;
								break;
								case Window::CursorArea::sizeS:
									grabType = Cursor::GrabbedWindow::GrabType::size_s;
								break;
								case Window::CursorArea::sizeSW:
									grabType = Cursor::GrabbedWindow::GrabType::size_sw;
								break;
								case Window::CursorArea::sizeSE:
									grabType = Cursor::GrabbedWindow::GrabType::size_se;
								break;
								case Window::CursorArea::sizeE:
									grabType = Cursor::GrabbedWindow::GrabType::size_e;
								break;
								case Window::CursorArea::sizeW:
									grabType = Cursor::GrabbedWindow::GrabType::size_w;
								break;
								case Window::CursorArea::none: // TODO: ignore this?
								case Window::CursorArea::titlebar:
								case Window::CursorArea::body:
									grabType = Cursor::GrabbedWindow::GrabType::move;
								break;
							}

							cursor->grab_window(*cursorWindow, grabType);
						}

					}else if(event.pressed.button==1){
						if(cursorWasDefault) cursor->set_cursor(&ui2d::image::cursors::_default_right, 0, 0);
					}
				} break;
				case Mouse::Event::Type::released:
					if(event.released.button==0||event.released.button==1){
						if(cursorWasDefault) cursor->set_cursor(&ui2d::image::cursors::_default, 0, 0);
					}

					if(event.released.button==0){
						if(cursor->grabbedWindow.window){
							cursor->release_window();
						}
					}
				case Mouse::Event::Type::scrolled:
				break;
				case Mouse::Event::Type::moved:
					if(cursor->grabbedWindow.window){
						if(!cursor->grabbedWindow.active&&maths::abs(cursor->x-cursor->grabbedWindow.startX)+maths::abs(cursor->y-cursor->grabbedWindow.startY)>=Cursor::GrabbedWindow::activationDistance){
							cursor->grabbedWindow.active = true;
						}

						if(cursor->grabbedWindow.active){
							auto &grabbedWindow = cursor->grabbedWindow;
							auto windowArea = DesktopManager::instance.get_window_area();

							switch(grabbedWindow.grabType){
								case Cursor::GrabbedWindow::GrabType::move:
									switch(grabbedWindow.window->state){
										case Window::State::minimised:
										break;
										case Window::State::docked:
											switch(grabbedWindow.window->dockedType){
												case Window::DockedType::top:
													if(cursor->y>=grabbedWindow.startY+(I32)edgeSnapDistance){
														grabbedWindow.window->restore();
														cursor->regrab_window_move();
													}
												break;
												case Window::DockedType::bottom:
													if(cursor->y<=grabbedWindow.startY-(I32)edgeSnapDistance){
														grabbedWindow.window->restore();
														cursor->regrab_window_move();
													}
												break;
												case Window::DockedType::left:
													if(cursor->x>=grabbedWindow.startX+(I32)edgeSnapDistance){
														grabbedWindow.window->restore();
														cursor->regrab_window_move();
													}
												break;
												case Window::DockedType::right:
													if(cursor->x<=grabbedWindow.startX-(I32)edgeSnapDistance){
														grabbedWindow.window->restore();
														cursor->regrab_window_move();
													}
												break;
												case Window::DockedType::full:
													if(cursor->y>=windowArea.y1+(I32)edgeSnapDistance){
														grabbedWindow.window->restore();
														cursor->regrab_window_move();
													}
												break;
											}
										break;
										case Window::State::floating:
											if(cursor->x<(windowArea.x1+windowArea.x2)/2&&cursor->x<windowArea.x1+(I32)edgeSnapDistance){
												grabbedWindow.window->dock(DesktopManager::Window::DockedType::left);
												cursor->regrab_window_move();
											}else if(cursor->x>=windowArea.x2-(I32)edgeSnapDistance){
												grabbedWindow.window->dock(DesktopManager::Window::DockedType::right);
												cursor->regrab_window_move();
											}else if(cursor->y<(windowArea.y1+windowArea.y2)/2&&cursor->y<windowArea.y1+(I32)edgeSnapDistance){
												grabbedWindow.window->dock(DesktopManager::Window::DockedType::top);
												cursor->regrab_window_move();
											}else if(cursor->y>=windowArea.y2-(I32)edgeSnapDistance){
												grabbedWindow.window->dock(DesktopManager::Window::DockedType::bottom);
												cursor->regrab_window_move();
											}else{
												grabbedWindow.window->move_to(cursor->x+grabbedWindow.grabOffsetX, cursor->y+grabbedWindow.grabOffsetY);
											}
										break;
									}
								break;
								case Cursor::GrabbedWindow::GrabType::size_n:
								case Cursor::GrabbedWindow::GrabType::size_nw:
								case Cursor::GrabbedWindow::GrabType::size_ne:
								case Cursor::GrabbedWindow::GrabType::size_s:
								case Cursor::GrabbedWindow::GrabType::size_sw:
								case Cursor::GrabbedWindow::GrabType::size_se:
								case Cursor::GrabbedWindow::GrabType::size_w:
								case Cursor::GrabbedWindow::GrabType::size_e: {
									const auto isLeft = grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_nw||grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_w||grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_sw;
									const auto isRight = grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_ne||grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_e||grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_se;
									const auto isTop = grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_nw||grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_n||grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_ne;
									const auto isBottom = grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_sw||grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_s||grabbedWindow.grabType==Cursor::GrabbedWindow::GrabType::size_se;

									const auto requestedWidth = (U32)maths::max(0, grabbedWindow.grabOffsetX+cursor->x*(isLeft?-1:isRight?+1:0));
									const auto requestedHeight = (U32)maths::max(0, grabbedWindow.grabOffsetY+cursor->y*(isTop?-1:isBottom?+1:0));

									grabbedWindow.window->events.trigger({
										type: Window::Event::Type::resizeRequested,
										resizeRequested: { requestedWidth, requestedHeight }
									});

									const auto newWidth = maths::clamp(requestedWidth, grabbedWindow.window->minSize.x, grabbedWindow.window->maxSize.x);
									const auto newHeight = maths::clamp(requestedHeight, grabbedWindow.window->minSize.y, grabbedWindow.window->maxSize.y);

									grabbedWindow.window->move_and_resize_to(
										grabbedWindow.window->get_x()-(isLeft?newWidth-grabbedWindow.window->get_width():0),
										grabbedWindow.window->get_y()-(isTop?newHeight-grabbedWindow.window->get_height():0),
										newWidth, newHeight
									);
								} break;
							}

							// if(cursor->dragWindow.window->state==DesktopManager::Window::State::docked){
							// 	auto hoveredWindow = get_window_at(cursor->x, cursor->y, false, cursor->display);
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

					}else{
						switch(cursorWindowArea){
							case Window::CursorArea::none:
							case Window::CursorArea::body:
							case Window::CursorArea::titlebar:
								cursor->set_cursor(&ui2d::image::cursors::_default, 0, 0);
							break;
							case Window::CursorArea::sizeW:
							case Window::CursorArea::sizeE:
								cursor->set_cursor(&ui2d::image::cursors::size_x, 12, 12);
							break;
							case Window::CursorArea::sizeN:
							case Window::CursorArea::sizeS:
								cursor->set_cursor(&ui2d::image::cursors::size_y, 12, 12);
							break;
							case Window::CursorArea::sizeNE:
							case Window::CursorArea::sizeSW:
								cursor->set_cursor(&ui2d::image::cursors::size_nesw, 12, 12);
							break;
							case Window::CursorArea::sizeNW:
							case Window::CursorArea::sizeSE:
								cursor->set_cursor(&ui2d::image::cursors::size_nwse, 12, 12);
							break;
						}
					}

					if(!cursor->isVisible){
						cursor->show();
					}
				break;
			}

			// update window under cursor
			if(window){
				switch(event.type){
					case driver::Mouse::Event::Type::moved: {
						if(cursor->lastHoveredWindow&&cursor->lastHoveredWindow!=window){
							cursor->lastHoveredWindow->_on_mouse_left();
							cursor->lastHoveredWindow->events.trigger({
								type: Window::Event::Type::mouseLeft,
								mouseLeft: {}
							});
						}
						window->_on_mouse_moved(cursor->x-window->graphicsDisplay->x, cursor->y-window->graphicsDisplay->y);
						window->events.trigger({
							type: Window::Event::Type::mouseMoved,
							mouseMoved: { event.instance, cursor->x-window->get_x(), cursor->y-window->get_y(), event.moved.x, event.moved.y }
						});
						cursor->lastHoveredWindow = window;
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

			}else{
				if(cursor->lastHoveredWindow){
					cursor->lastHoveredWindow->_on_mouse_left();
					cursor->lastHoveredWindow->events.trigger({
						type: Window::Event::Type::mouseLeft,
						mouseLeft: {}
					});

					cursor->lastHoveredWindow = nullptr;
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

	void StandardWindow::_on_graphicsDisplay_resized() {
		if(enableTransparency){
			graphicsDisplay->buffer.draw_rect((graphics2d::Rect){0,0,(I32)graphicsDisplay->buffer.width,(I32)graphicsDisplay->buffer.height}, 0xff000000);
		}

		const auto clientAreaRect = theme.get_window_client_area({0, 0, (I32)graphicsDisplay->buffer.width, (I32)graphicsDisplay->buffer.height});
		clientArea = graphicsDisplay->buffer.region(clientAreaRect.x1, clientAreaRect.y1, clientAreaRect.width(), clientAreaRect.height());
		clientArea.draw_rect(0, 0, clientArea.width, clientArea.height, windowBackgroundColour);
		gui.buffer = graphicsDisplay->buffer;

		_set_titlebar_area(theme.get_window_titlebar_area({0, 0, (I32)graphicsDisplay->buffer.width, (I32)graphicsDisplay->buffer.height}));

		_draw_frame();

		_update_window_area();

		events.trigger({
			type: DesktopManager::Window::Event::Type::clientAreaChanged
		});

		graphicsDisplay->update();
	}

	void StandardWindow::resize_to(U32 width, U32 height) {
		width = maths::clamp(width, minSize.x, maxSize.x);
		height = maths::clamp(height, minSize.y, maxSize.y);

		auto oldWidth = get_width();
		auto oldHeight = get_height();
		graphicsDisplay->resize_to(leftMargin+width+rightMargin, topMargin+height+bottomMargin);

		if(get_width()==oldWidth&&get_height()==oldHeight) return;

		_on_graphicsDisplay_resized();
	}

	void StandardWindow::move_and_resize_to(I32 x, I32 y, U32 width, U32 height) {
		isAutomaticPlacement = false;

		width = maths::clamp(width, minSize.x, maxSize.x);
		height = maths::clamp(height, minSize.y, maxSize.y);

		auto oldWidth = get_width();
		auto oldHeight = get_height();

		if(width==oldWidth&&height==oldHeight) return move_to(x, y);

		graphicsDisplay->move_and_resize_to({x-(I32)leftMargin, y-(I32)topMargin, x+(I32)width+(I32)rightMargin, y+(I32)height+(I32)bottomMargin});

		clientArea = graphicsDisplay->buffer.cropped(leftMargin, topMargin, rightMargin, bottomMargin);

		_on_graphicsDisplay_resized();
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

		window._draw_frame();

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
