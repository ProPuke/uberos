#include "taskbar.hpp"

#include <drivers/Clock.hpp>
#include <drivers/DesktopManager.hpp>
#include <drivers/Keyboard.hpp>
#include <drivers/Mouse.hpp>
#include <drivers/ThemeManager.hpp>

#include <kernel/drivers.hpp>

#include <common/ui2d/control/Area.hpp>
#include <common/ui2d/control/Button.hpp>
#include <common/ui2d/control/ColouredButton.hpp>
#include <common/ui2d/controlContainer/Box.hpp>
#include <common/graphics2d.hpp>
#include <common/graphics2d/font.hpp>

#include <functional>

#define PREVIEW_INCLUDE_BORDER

namespace ui2d::image {
	namespace icons {
		extern graphics2d::MultisizeIcon super;
	}
}

namespace tests::taskbar {
	namespace {
		struct WindowButton;

		driver::DesktopManager *desktopManager = nullptr;
		PodArray<ui2d::LayoutControl<WindowButton>*> windowButtons;
		driver::DesktopManager::Window *lastFocusedWindow = nullptr;
		ui2d::Theme *theme = nullptr;

		DriverReference<driver::ThemeManager> themeManager{nullptr, [](void*){
			themeManager = drivers::find_and_activate<driver::ThemeManager>();
			if(!themeManager) {
				//TODO: terminate?
				return;
			}

			theme = &themeManager->get_theme();
			themeManager->events.subscribe([](const driver::ThemeManager::Event &event) {
				if(event.type==driver::ThemeManager::Event::Type::themeChanged){
					theme = &themeManager->get_theme();
				}
			});
		}, nullptr};

		const auto width = 600;
		const auto height = 150;
		U32 transparentBackgroundColour;
		U32 opaqueBackgroundColour;
		// const auto borderColour = desktopManager->get_default_window_border_colour();
		const auto borderColour = graphics2d::premultiply_colour(0xeeeeee|(255-0x88)<<24);
		// const auto opaqueBorderColour = graphics2d::premultiply_colour(0xeeeeee|(255-0xdd)<<24);
		const auto opaqueBorderColour = desktopManager->get_default_window_border_colour();

		auto maxButtonWidth = 200;
		auto maxButtonHeight = 50;
		auto minButtonHeight = 20;

		auto isVertical = false;

		driver::DesktopManager::CustomWindow *taskbarWindow;
		driver::DesktopManager::CustomWindow *previewWindow;

		void show_button_preview(WindowButton*);

		struct DesktopGui: ui2d::Gui {
			typedef ui2d::Gui Super;
	
			driver::DesktopManager::Window &window;
	
			/**/ DesktopGui(driver::DesktopManager::Window &window);
	
			void update_area(graphics2d::Rect area) override;

			void on_mouse_left() override;
			void on_mouse_moved(I32 x, I32 y) override;
		};

		struct WindowButton: ui2d::control::ColouredButton {
			typedef ui2d::control::ColouredButton Super;
	
			driver::DesktopManager::Window &window;

			/**/ WindowButton(ui2d::Gui &gui, graphics2d::Rect rect, driver::DesktopManager::Window &window);
	
			void on_pressed() override;
			void on_clicked() override;
		};

		/**/ DesktopGui::DesktopGui(driver::DesktopManager::Window &window):
			Super(window.get_client_buffer(), taskbar::theme),
			window(window)
		{}

		void DesktopGui::update_area(graphics2d::Rect area) {
			if(isFrozen) return;
			window.redraw_area(area);
		}

		void DesktopGui::on_mouse_left() {
			Super::on_mouse_left();

			show_button_preview(nullptr);
		}

		void DesktopGui::on_mouse_moved(I32 x, I32 y) {
			Super::on_mouse_moved(x, y);

			auto previewShown = false;

			for(auto button:windowButtons){
				if(button->rect.contains(x, y)){
					previewShown = true;
					show_button_preview(button);
					break;
				}
			}

			if(!previewShown){
				show_button_preview(nullptr);
			}
		}

		/**/ WindowButton::WindowButton(ui2d::Gui &gui, graphics2d::Rect rect, driver::DesktopManager::Window &window):
			Super(gui, rect, graphics2d::premultiply_colour(desktopManager->get_default_window_colour()|0x55<<24), window.get_title()),
			window(window)
		{
			set_toggle(false);
		}

		void WindowButton::on_pressed() {
			// keep the window focused if it was already (rather than losing focus to the taskbar)
			if(this->toggleActive){
				window.focus();
			}
		}

		void WindowButton::on_clicked() {
			switch(window.get_state()){
				case driver::DesktopManager::Window::State::docked:
				case driver::DesktopManager::Window::State::floating:
					if(&window==lastFocusedWindow&&window.is_top()){
						window.minimise();
					}else{
						window.focus();
					}
				break;
				case driver::DesktopManager::Window::State::minimised:
					window.focus();
				break;
			}
		}

		void update_windowButton_states() {
			for(auto windowButton:windowButtons){
				windowButton->set_toggle(&windowButton->window==lastFocusedWindow);
			}
		}

		static WindowButton *currentPreview = nullptr;

		void show_button_preview(WindowButton *button) {
			if(currentPreview==button) return;
			currentPreview = button;

			if(!previewWindow) return;

			previewWindow->hide();

			if(!button) return;

			#ifdef PREVIEW_INCLUDE_BORDER
				auto &clientArea = button->window.get_window_buffer();
			#else
				auto &clientArea = button->window.get_client_area();
			#endif

			#ifdef PREVIEW_INCLUDE_BORDER
				U32 corner[2];
			#else
				U32 corner[3];
			#endif
			graphics2d::create_diagonal_corner(sizeof(corner)/sizeof(corner[0])-1, corner);
			previewWindow->set_corner(corner, corner, corner, corner);

			I32 width;
			I32 height;

			{
				const auto x = taskbarWindow->get_x();
				const auto y = taskbarWindow->get_y();
				const auto windowArea = desktopManager->get_window_area();

				if(isVertical){
					width = button->rect.width();
					height = width*clientArea.height/clientArea.width;

					#ifdef PREVIEW_INCLUDE_BORDER
						previewWindow->set_margin(
							button->window.get_left_margin()*width/clientArea.width,
							button->window.get_top_margin()*height/clientArea.height,
							button->window.get_right_margin()*width/clientArea.width,
							button->window.get_bottom_margin()*height/clientArea.height
						);
					#endif

					if(x<(windowArea.x1+windowArea.x2)/2){
						// taskbar at left, preview on right
						previewWindow->move_and_resize_to(x+button->rect.x2+10, y+button->rect.y1-(height-button->rect.height())/2, width, height);
						previewWindow->move_and_resize_to(x+button->rect.x2+10, y+button->rect.y1-(height-button->rect.height())/2, width, height);
					}else{
						// taskbar at right, preview on left
						previewWindow->move_and_resize_to(x+button->rect.x1-10-width, y+button->rect.y1-(height-button->rect.height())/2, width, height);
						previewWindow->move_and_resize_to(x+button->rect.x1-10-width, y+button->rect.y1-(height-button->rect.height())/2, width, height);
					}

				}else{
					width = button->rect.width();
					height = width*clientArea.height/clientArea.width;

					#ifdef PREVIEW_INCLUDE_BORDER
						previewWindow->set_margin(
							button->window.get_left_margin()*width/clientArea.width,
							button->window.get_top_margin()*height/clientArea.height,
							button->window.get_right_margin()*width/clientArea.width,
							button->window.get_bottom_margin()*height/clientArea.height
						);
					#endif

					if(y<(windowArea.y1+windowArea.y2)/2){
						// taskbar at top, preview below
						previewWindow->move_and_resize_to(x+button->rect.x1, y+button->rect.y2+10, width, height);
						previewWindow->move_and_resize_to(x+button->rect.x1, y+button->rect.y2+10, width, height);
					}else{
						// taskbar below, preview above
						previewWindow->move_and_resize_to(x+button->rect.x1, y+button->rect.y1-10-height, width, height);
						previewWindow->move_and_resize_to(x+button->rect.x1, y+button->rect.y1-10-height, width, height);
					}
				}
			}

			#ifdef PREVIEW_INCLUDE_BORDER
				previewWindow->set_solid_area({0,0,0,0});
			#else
				previewWindow->set_solid_area({0,0,width,height});
			#endif
			previewWindow->set_interact_area({0,0,0,0});

			auto previewWindowArea = previewWindow->get_window_buffer();
			#ifndef PREVIEW_INCLUDE_BORDER
				previewWindowArea.draw_scaled_buffer(0, 0, previewWindowArea.width, previewWindowArea.height, clientArea, 0, 0, clientArea.width, clientArea.height, {});
				previewWindowArea.draw_rect_outline({0,0,(I32)previewWindowArea.width,(I32)previewWindowArea.height}, 0x888888, 1, corner, corner, corner, corner);
			#else
				previewWindowArea.draw_scaled_buffer(0, 0, previewWindowArea.width, previewWindowArea.height, clientArea, 0, 0, clientArea.width, clientArea.height, {});
				if(previewWindowArea.width<=clientArea.width/2&&previewWindowArea.height<=clientArea.height/2){
					previewWindowArea.draw_rect_outline({(I32)previewWindow->get_left_margin(),(I32)previewWindow->get_top_margin(),(I32)previewWindowArea.width-(I32)previewWindow->get_right_margin(),(I32)previewWindowArea.height-(I32)previewWindow->get_bottom_margin()}, 0x888888, 1, corner, corner, corner, corner);
				}
			#endif
			previewWindow->redraw();
			previewWindow->show();
		}
	}

	void run() {
		desktopManager = drivers::find_and_activate<driver::DesktopManager>();
		if(!desktopManager) return;

		themeManager = drivers::find_and_activate<driver::ThemeManager>();
		if(!themeManager) return;
		theme = &themeManager->get_theme();

		transparentBackgroundColour = graphics2d::premultiply_colour(desktopManager->get_default_window_colour()|(0xff-0x44)<<24);
		opaqueBackgroundColour = graphics2d::premultiply_colour(desktopManager->get_default_window_colour()|0x44<<24);
		minButtonHeight = theme->get_minimum_button_height()*2/3;

		lastFocusedWindow = desktopManager->get_focused_window();

		const static auto shadowLength = 8u;

		taskbarWindow = &desktopManager->create_custom_window("Taskbar", width, height+shadowLength);
		taskbarWindow->set_margin(shadowLength, shadowLength, shadowLength, shadowLength);
		taskbarWindow->set_interact_area({shadowLength,shadowLength,shadowLength+width,shadowLength+height});
		taskbarWindow->set_titlebar_area({shadowLength,shadowLength,shadowLength+width,shadowLength+height});
		taskbarWindow->set_max_docked_size(150, 61);
		taskbarWindow->dock(driver::DesktopManager::Window::DockedType::top);
		taskbarWindow->set_layer(driver::DesktopManager::Window::Layer::topmost);

		previewWindow = &desktopManager->create_custom_window("Taskbar preview", 16, 16);
		previewWindow->set_layer(driver::DesktopManager::Window::Layer::topmost);

		static DesktopGui gui{*taskbarWindow};
		gui.backgroundColour = transparentBackgroundColour;

		const auto padding = 5;

		static ui2d::controlContainer::Box box{gui};
		static auto &launcherButton = box.add_control<ui2d::control::ColouredButton>(0xff000000, "");
		static auto &buttonContainer = box.add_container_control<ui2d::controlContainer::Box>();
		buttonContainer.set_alignment(0.5f);

		static auto &clockArea = box.add_control<ui2d::control::Area>();

		launcherButton.icon = ui2d::image::icons::super;
		launcherButton.set_min_size(theme->get_minimum_button_width()/2, minButtonHeight);

		static auto _set_orientation = [](bool vertical) {
			isVertical = vertical;
			static auto active = false;
			if(active) return; // prevent recursion (_set_orientation -> set_*_size -> redraw -> _set_orientation)
			active = true;

			if(vertical){
				box.set_direction(ui2d::controlContainer::Box::Direction::vertical);
				box.set_expand(0.0, 1.0);
				buttonContainer.set_direction(ui2d::controlContainer::Box::Direction::vertical);
				buttonContainer.set_expand(0.0, 1.0);
			}else{
				box.set_direction(ui2d::controlContainer::Box::Direction::horizontal);
				box.set_expand(1.0, 0.0);
				buttonContainer.set_direction(ui2d::controlContainer::Box::Direction::horizontal);
				buttonContainer.set_expand(1.0, 0.0);
			}

			{
				auto minSize = box.get_min_size();
				auto maxSize = box.get_max_size();
				taskbarWindow->set_size_limits(
					minSize.x + padding*2, minSize.y + padding*2,
					maths::add_safe(maxSize.x, padding*2), maths::add_safe(maxSize.y, padding*2)
				);
			}

			active = false;
		};

		static auto redraw = [](){
			{ auto guiFreeze = gui.freeze();
				auto width = taskbarWindow->get_width();
				auto height = taskbarWindow->get_height();
				auto &clientArea = taskbarWindow->get_client_buffer();
				auto &windowArea = taskbarWindow->get_window_buffer();

				clientArea.draw_rect(0, 0, clientArea.width, clientArea.height, 0xff000000);

				if(taskbarWindow->get_state()==driver::DesktopManager::Window::State::docked){
					launcherButton.colour = 0xff000000;

					taskbarWindow->set_solid_area({0,0,0,0});
					taskbarWindow->set_titlebar_area({0, 0, (I32)width, (I32)height});

					// windowArea.draw_rect(0, 0, width, height, transparentBackgroundColour);
					clientArea.draw_rect(0, 0, width, height, transparentBackgroundColour);

					switch(taskbarWindow->get_docked_type()){
						case driver::DesktopManager::Window::DockedType::top:
							windowArea.set(shadowLength, shadowLength+height-1, borderColour, width);
							for(auto i=0u;i<shadowLength;i++){
								windowArea.set(shadowLength, shadowLength+height+i, 0x000000|(255-20+20*i/(shadowLength-1))<<24, width);
							}
						break;
						case driver::DesktopManager::Window::DockedType::bottom:
							windowArea.set(shadowLength, shadowLength, borderColour, width);
							for(auto i=0u;i<shadowLength;i++){
								windowArea.set(0u, i, 0x000000|(255-20*i/(shadowLength-1))<<24, width);
							}
						break;
						case driver::DesktopManager::Window::DockedType::left:
							windowArea.draw_line(shadowLength+width-1, shadowLength, shadowLength+width-1, shadowLength+height-1, borderColour);
							for(auto i=0u;i<shadowLength;i++){
								windowArea.draw_line(
									shadowLength+width+i, shadowLength+0,
									shadowLength+width+i, shadowLength+height-1,
									0x000000|(255-20+20*i/(shadowLength-1))<<24
								);
							}
						break;
						case driver::DesktopManager::Window::DockedType::right:
							windowArea.draw_line(shadowLength, shadowLength, shadowLength, shadowLength+height-1, borderColour);
							for(auto i=0u;i<shadowLength;i++){
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
					launcherButton.colour = graphics2d::premultiply_colour(0x55eeeeee);

					// window->set_solid_area({shadowLength, shadowLength, shadowLength+width, shadowLength+height});
					// clientArea.draw_rect(1, 1, width-1, height-1, solidBackgroundColour);

					taskbarWindow->set_solid_area({0,0,0,0});
					taskbarWindow->set_titlebar_area({0, 0, (I32)width, (I32)height});
					clientArea.draw_rect(0, 0, width, height, opaqueBackgroundColour);

					clientArea.draw_rect_outline(0, 0, width, height, opaqueBorderColour);

					for(auto i=0u;i<shadowLength;i++){
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

					auto smallFontSettings = graphics2d::Buffer::FontSettings{
						.font = graphics2d::font::manrope_extraBold,
						.size = 36
					};
					auto largeFontSettings = graphics2d::Buffer::FontSettings{
						.font = graphics2d::font::manrope_extraBold,
						.size = 48
					};

					box.set_rect({padding, padding, (I32)clientArea.width-padding, (I32)clientArea.height-padding});

					auto smallFontMeasurements = clientArea.measure_text(smallFontSettings, time);
					auto largeFontMeasurements = clientArea.measure_text(largeFontSettings, time);

					auto smallWidth = (U32)smallFontMeasurements.rect.width()+1;
					auto largeWidth = (U32)largeFontMeasurements.rect.width()+1;
					auto smallHeight = (U32)smallFontMeasurements.capHeight+3;
					auto largeHeight = (U32)largeFontMeasurements.capHeight+3;
					
					clockArea.set_min_size(smallWidth, smallHeight);
					clockArea.set_max_size(largeWidth, largeHeight);

					_set_orientation(clientArea.height>clientArea.width);

					if(clientArea.width>=clientArea.height){
						if(clockArea.rect.width()>=(I32)largeWidth&&clockArea.rect.height()>=(I32)largeHeight){
							clientArea.draw_text(largeFontSettings, time, clockArea.rect.x1+1, clockArea.rect.y2-3+1, largeWidth, 0x000000);
							clientArea.draw_text(largeFontSettings, time, clockArea.rect.x1, clockArea.rect.y2-3, largeWidth, 0xffffff);
						}else{
							clientArea.draw_text(smallFontSettings, time, clockArea.rect.x2-smallWidth+1, clockArea.rect.y2/2-1+smallHeight/2+1, smallWidth, 0x000000);
							clientArea.draw_text(smallFontSettings, time, clockArea.rect.x2-smallWidth, clockArea.rect.y2/2-1+smallHeight/2, smallWidth, 0xffffff);
						}
					}else{
						if(clockArea.rect.width()>=(I32)largeWidth&&clockArea.rect.height()>=(I32)largeHeight){
							clientArea.draw_text(largeFontSettings, time, clockArea.rect.x1+1, clockArea.rect.y2-3+1, largeWidth, 0x000000);
							clientArea.draw_text(largeFontSettings, time, clockArea.rect.x1, clockArea.rect.y2-3, largeWidth, 0xffffff);
						}else{
							clientArea.draw_text(smallFontSettings, time, (clockArea.rect.x1+(clockArea.rect.x2-smallWidth))/2+1, clockArea.rect.y2-3+1, smallWidth, 0x000000);
							clientArea.draw_text(smallFontSettings, time, (clockArea.rect.x1+(clockArea.rect.x2-smallWidth))/2, clockArea.rect.y2-3, smallWidth, 0xffffff);
						}
					}
				}

				for(auto button:windowButtons){
					button->set_small_font(button->rect.width()<(I32)theme->get_minimum_button_width()*4/3);
				}

				gui.buffer = clientArea;
				if(clientArea.width>=clientArea.height){
					launcherButton.set_max_size((I32)theme->get_minimum_button_width(), maths::min(maxButtonHeight, padding+(I32)clientArea.height-padding-padding));
				}else{
					launcherButton.set_max_size(maths::min(maxButtonWidth, (I32)clientArea.width-padding-padding), padding+(I32)(theme->get_minimum_button_width()*1/3+minButtonHeight));
				}
			}

			gui.redraw(false);
		};

		{
			auto i=0u;
			for(auto desktopWindow=desktopManager->get_window(i); desktopWindow; desktopWindow=desktopManager->get_window(++i)){
				if(desktopWindow==taskbarWindow) continue;
				if(desktopWindow==previewWindow) continue;
				if(!desktopWindow->is_visible()) continue;

				auto &control = windowButtons.push_back(&buttonContainer.add_control<WindowButton>(std::ref(*desktopWindow)));
				control->set_min_size(theme->get_minimum_button_width()*7/12, minButtonHeight);
				control->set_max_size(maxButtonWidth, maxButtonHeight);
			}
		}

		update_windowButton_states();

		redraw();

		desktopManager->events.subscribe([](const driver::DesktopManager::Event &event){
			if(event.type==driver::DesktopManager::Event::Type::windowShown){
				if(event.windowShown.window==taskbarWindow) return;
				if(event.windowShown.window==previewWindow) return;

				auto &control = windowButtons.push_back(&buttonContainer.add_control<WindowButton>(std::ref(*event.windowShown.window)));
				control->set_min_size(theme->get_minimum_button_width()*7/12, minButtonHeight);
				control->set_max_size(maxButtonWidth, maxButtonHeight);
				redraw();
				taskbarWindow->redraw();

			}else if(event.type==driver::DesktopManager::Event::Type::windowHidden||event.type==driver::DesktopManager::Event::Type::windowRemoved){
				auto removedWindow = event.type==driver::DesktopManager::Event::Type::windowHidden?event.windowHidden.window:event.windowRemoved.window;

				if(removedWindow==lastFocusedWindow){
					lastFocusedWindow = nullptr;
				}

				for(auto i=0u;i<windowButtons.length;i++){
					if(&windowButtons[i]->window==removedWindow){
						buttonContainer.remove_control(*windowButtons[i]);
						windowButtons.remove(i);
						redraw();
						taskbarWindow->redraw();
						break;
					}
				}

			}else if(event.type==driver::DesktopManager::Event::Type::windowFocused){
				if(event.windowFocused.window==taskbarWindow) return; // ignore focusing the taskbar itself
				if(event.windowFocused.window==previewWindow) return;

				lastFocusedWindow = event.windowFocused.window;
				update_windowButton_states();
			}
		});

		taskbarWindow->events.subscribe([](const driver::DesktopManager::Window::Event &event){
			if(event.type==driver::DesktopManager::Window::Event::Type::resizeRequested){
				_set_orientation(event.resizeRequested.width<event.resizeRequested.height);

			}else if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
				redraw();

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseLeft){
				gui.on_mouse_left();

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseMoved){
				gui.on_mouse_moved(event.mouseMoved.x, event.mouseMoved.y);

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mousePressed){
				gui.on_mouse_pressed(event.mousePressed.x, event.mousePressed.y, event.mousePressed.button);

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseReleased){
				gui.on_mouse_released(event.mouseReleased.x, event.mouseReleased.y, event.mouseReleased.button);
			}
		});

		taskbarWindow->show();
	}
}
