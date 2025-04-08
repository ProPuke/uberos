#include "taskbar.hpp"

#include <drivers/Clock.hpp>
#include <drivers/DesktopManager.hpp>
#include <drivers/Keyboard.hpp>
#include <drivers/Mouse.hpp>

#include <kernel/drivers.hpp>

#include <common/ui2d/control/Area.hpp>
#include <common/ui2d/control/Button.hpp>
#include <common/ui2d/control/ColouredButton.hpp>
#include <common/ui2d/theme/Clean.hpp>
#include <common/ui2d/controlContainer/Box.hpp>
#include <common/graphics2d.hpp>
#include <common/graphics2d/font.hpp>

#include <functional>

namespace ui2d::image {
	namespace icons {
		extern graphics2d::Buffer super;
	}
}

namespace tests::taskbar {
	namespace {
		struct WindowButton;

		driver::DesktopManager *desktopManager = nullptr;
		PodArray<ui2d::LayoutControl<WindowButton>*> windowButtons;
		driver::DesktopManager::Window *lastFocusedWindow = nullptr;

		const auto width = 600;
		const auto height = 150;
		const auto transparentBackgroundColour = graphics2d::premultiply_colour(desktopManager->get_default_window_colour()|(0xff-0x44)<<24);
		const auto opaqueBackgroundColour = graphics2d::premultiply_colour(desktopManager->get_default_window_colour()|0x44<<24);
		// const auto borderColour = desktopManager->get_default_window_border_colour();
		const auto borderColour = graphics2d::premultiply_colour(0xeeeeee|(255-0x88)<<24);
		// const auto opaqueBorderColour = graphics2d::premultiply_colour(0xeeeeee|(255-0xdd)<<24);
		const auto opaqueBorderColour = desktopManager->get_default_window_border_colour();

		static ui2d::theme::Clean cleanTheme;

		const auto maxButtonWidth = 200;
		const auto maxButtonHeight = 50;
		const auto minButtonHeight = cleanTheme.get_minimum_button_height()*2/3;

		struct DesktopGui: ui2d::Gui {
			typedef ui2d::Gui Super;
	
			ui2d::theme::Clean theme;
			driver::DesktopManager::Window &window;
	
			/**/ DesktopGui(driver::DesktopManager::Window &window);
	
			void update_area(graphics2d::Rect area) override;
		};

		struct WindowButton: ui2d::control::ColouredButton {
			typedef ui2d::control::ColouredButton Super;
	
			driver::DesktopManager::Window &window;

			/**/ WindowButton(ui2d::Gui &gui, graphics2d::Rect rect, driver::DesktopManager::Window &window);
	
			void on_pressed() override;
			void on_clicked() override;
		};

		/**/ DesktopGui::DesktopGui(driver::DesktopManager::Window &window):
			Super(window.get_client_area(), theme),
			window(window)
		{}

		void DesktopGui::update_area(graphics2d::Rect area) {
			if(isFrozen) return;
			window.redraw_area(area);
		}

		/**/ WindowButton::WindowButton(ui2d::Gui &gui, graphics2d::Rect rect, driver::DesktopManager::Window &window):
			Super(gui, rect, graphics2d::premultiply_colour(0x55dddddd), window.get_title()),
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
	}

	void run() {
		desktopManager = drivers::find_and_activate<driver::DesktopManager>();
		if(!desktopManager) return;

		lastFocusedWindow = desktopManager->get_focused_window();

		const static auto shadowLength = 8;

		static auto window = &desktopManager->create_custom_window("Taskbar", width, height+shadowLength);
		window->set_margin(shadowLength, shadowLength, shadowLength, shadowLength);
		window->set_interact_area({shadowLength,shadowLength,shadowLength+width,shadowLength+height});
		window->set_titlebar_area({shadowLength,shadowLength,shadowLength+width,shadowLength+height});
		window->set_max_docked_size(150, 61);
		window->dock(driver::DesktopManager::Window::DockedType::top);
		window->set_layer(driver::DesktopManager::Window::Layer::topmost);
		window->show();

		static DesktopGui gui{*window};
		gui.backgroundColour = transparentBackgroundColour;

		const auto padding = 5;

		static ui2d::controlContainer::Box box{gui};
		static auto &launcherButton = box.add_control<ui2d::control::ColouredButton>(0xff000000, "");
		static auto &buttonContainer = box.add_container_control<ui2d::controlContainer::Box>();
		buttonContainer.set_alignment(0.5f);

		static auto &clockArea = box.add_control<ui2d::control::Area>();

		launcherButton.icon = &ui2d::image::icons::super;
		launcherButton.set_min_size(cleanTheme.get_minimum_button_width()/2, minButtonHeight);

		static auto _set_orientation = [](bool vertical) {
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
				window->set_min_size(maths::add_safe(minSize.x, (U32)padding*2u), maths::add_safe(minSize.y, (U32)padding*2u), false);
				window->set_max_size(maths::add_safe(maxSize.x, (U32)padding*2u), maths::add_safe(maxSize.y, (U32)padding*2u), false);
			}

			active = false;
		};

		static auto redraw = [](){
			{ auto guiFreeze = gui.freeze();
				auto width = window->get_width();
				auto height = window->get_height();
				auto &clientArea = window->get_client_area();
				auto &windowArea = window->get_window_area();

				clientArea.draw_rect(0, 0, clientArea.width, clientArea.height, 0xff000000);

				if(window->get_state()==driver::DesktopManager::Window::State::docked){
					launcherButton.colour = 0xff000000;

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
					launcherButton.colour = graphics2d::premultiply_colour(0x55eeeeee);

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

					auto smallWidth = (U32)smallFontMeasurements.blockWidth+1;
					auto largeWidth = (U32)largeFontMeasurements.blockWidth+1;
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
					button->set_small_font(button->rect.width()<(I32)cleanTheme.get_minimum_button_width()*4/3);
				}

				gui.buffer = clientArea;
				if(clientArea.width>=clientArea.height){
					launcherButton.set_max_size((I32)cleanTheme.get_minimum_button_width(), maths::min(maxButtonHeight, padding+(I32)clientArea.height-padding-padding));
				}else{
					launcherButton.set_max_size(maths::min(maxButtonWidth, (I32)clientArea.width-padding-padding), padding+(I32)(cleanTheme.get_minimum_button_width()*1/3+minButtonHeight));
				}
			}

			gui.redraw(false);
		};

		{
			auto i=0u;
			for(auto desktopWindow=desktopManager->get_window(i); desktopWindow; desktopWindow=desktopManager->get_window(++i)){
				if(desktopWindow==window) continue;
				auto &control = windowButtons.push_back(&buttonContainer.add_control<WindowButton>(std::ref(*desktopWindow)));
				control->set_min_size(cleanTheme.get_minimum_button_width()*7/12, minButtonHeight);
				control->set_max_size(maxButtonWidth, maxButtonHeight);
			}
		}

		update_windowButton_states();

		redraw();
		window->redraw();

		desktopManager->events.subscribe([](const driver::DesktopManager::Event &event){
			if(event.type==driver::DesktopManager::Event::Type::windowAdded){
				auto &control = windowButtons.push_back(&buttonContainer.add_control<WindowButton>(std::ref(*event.windowAdded.window)));
				control->set_min_size(cleanTheme.get_minimum_button_width()*7/12, minButtonHeight);
				control->set_max_size(maxButtonWidth, maxButtonHeight);
				redraw();
				window->redraw();

			}else if(event.type==driver::DesktopManager::Event::Type::windowRemoved){
				if(event.windowRemoved.window==lastFocusedWindow){
					lastFocusedWindow = nullptr;
				}

				for(auto i=0u;i<windowButtons.length;i++){
					if(&windowButtons[i]->window==event.windowRemoved.window){
						buttonContainer.remove_control(*windowButtons[i]);
						windowButtons.remove(i);
						redraw();
						window->redraw();
						break;
					}
				}

			}else if(event.type==driver::DesktopManager::Event::Type::windowFocused){
				if(event.windowFocused.window==window) return; // ignore focusing the taskbar itself

				lastFocusedWindow = event.windowFocused.window;
				update_windowButton_states();
			}
		});

		window->events.subscribe([](const driver::DesktopManager::Window::Event &event){
			if(event.type==driver::DesktopManager::Window::Event::Type::resizeRequested){
				_set_orientation(event.resizeRequested.width<event.resizeRequested.height);

			}else if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
				redraw();

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseMoved){
				gui.on_mouse_moved(event.mouseMoved.x, event.mouseMoved.y);

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mousePressed){
				gui.on_mouse_pressed(event.mousePressed.x, event.mousePressed.y, event.mousePressed.button);

			}else if(event.type==driver::DesktopManager::Window::Event::Type::mouseReleased){
				gui.on_mouse_released(event.mouseReleased.x, event.mouseReleased.y, event.mouseReleased.button);
			}
		});
	}
}
