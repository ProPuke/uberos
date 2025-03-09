#include "taskbar.hpp"

#include <drivers/Clock.hpp>
#include <drivers/DesktopManager.hpp>
#include <drivers/Keyboard.hpp>
#include <drivers/Mouse.hpp>

#include <kernel/drivers.hpp>

#include <common/ui2d/control/Button.hpp>
#include <common/ui2d/control/ColouredButton.hpp>
#include <common/ui2d/theme/Clean.hpp>
#include <common/graphics2d.hpp>
#include <common/graphics2d/font.hpp>

namespace ui2d::image {
	namespace icons {
		extern graphics2d::Buffer super;
	}
}

namespace tests::taskbar {
	namespace {
		struct WindowButton;

		driver::DesktopManager *desktopManager = nullptr;
		PodArray<WindowButton*> windowButtons;

		const auto width = 600;
		const auto height = 150;
		const auto transparentBackgroundColour = graphics2d::premultiply_colour(desktopManager->get_default_window_colour()|(0xff-0x44)<<24);
		const auto opaqueBackgroundColour = graphics2d::premultiply_colour(desktopManager->get_default_window_colour()|0x44<<24);
		// const auto borderColour = desktopManager->get_default_window_border_colour();
		const auto borderColour = graphics2d::premultiply_colour(0xeeeeee|(255-0x88)<<24);
		const auto opaqueBorderColour = graphics2d::premultiply_colour(0xeeeeee|(255-0xdd)<<24);

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
			if(this->toggleActive){
				window.focus();
			}
		}

		void update_windowButton_states() {
			auto focusedWindow = desktopManager->get_focused_window();
			for(auto windowButton:windowButtons){
				windowButton->set_toggle(&windowButton->window==focusedWindow);
			}
		}
	}

	void run() {
		desktopManager = drivers::find_and_activate<driver::DesktopManager>();
		if(!desktopManager) return;

		const static auto shadowLength = 8;

		static auto window = &desktopManager->create_custom_window("Taskbar", width, height+shadowLength);
		window->set_margin(shadowLength, shadowLength, shadowLength, shadowLength);
		window->set_interact_area({shadowLength,shadowLength,shadowLength+width,shadowLength+height});
		window->set_titlebar_area({shadowLength,shadowLength,shadowLength+width,shadowLength+height});
		window->set_max_docked_size(150, 61);
		window->dock(driver::DesktopManager::Window::DockedType::top);
		window->set_layer(driver::DesktopManager::Window::Layer::topmost);

		static DesktopGui gui{*window};
		gui.backgroundColour = transparentBackgroundColour;

		const auto padding = 5;

		static ui2d::theme::Clean cleanTheme;

		static ui2d::control::ColouredButton launcherButton{gui, {padding, padding, padding, padding}, 0xff000000, ""};
		launcherButton.icon = &ui2d::image::icons::super;

		static auto layoutWindowButtons = [](){
			auto &clientArea = window->get_client_area();

			if(clientArea.width>=clientArea.height){
				const auto margin = 69; // big enough for the clock
				const auto innerWidth = (I32)clientArea.width-margin*2;

				const auto maxButtonWidth = 150;

				const auto buttonSpacing = padding;
				const auto buttonsWidth = min(innerWidth, (I32)windowButtons.length*(maxButtonWidth+buttonSpacing)-buttonSpacing);

				const auto left = margin+innerWidth/2-buttonsWidth/2;

				for(auto i=0u;i<windowButtons.length;i++){
					auto &windowButton = *windowButtons[i];
					windowButton.rect = {
						left+(I32)((buttonsWidth+buttonSpacing)*(i/(float)windowButtons.length)), padding,
						left+(I32)((buttonsWidth+buttonSpacing)*((i+1)/(float)windowButtons.length)-buttonSpacing), (I32)clientArea.height-padding
					};
				}

			}else{
				const auto margin = 63; // big enough for the clock
				const auto innerHeight = (I32)clientArea.height-margin*2;

				const auto maxButtonHeight = 40;

				const auto buttonSpacing = padding;
				const auto buttonsHeight = min(innerHeight, (I32)windowButtons.length*(maxButtonHeight+buttonSpacing)-buttonSpacing);

				const auto top = margin+innerHeight/2-buttonsHeight/2;

				for(auto i=0u;i<windowButtons.length;i++){
					auto &windowButton = *windowButtons[i];
					windowButton.rect = {
						padding, top+(I32)((buttonsHeight+buttonSpacing)*(i/(float)windowButtons.length)),
						(I32)clientArea.width-padding, top+(I32)((buttonsHeight+buttonSpacing)*((i+1)/(float)windowButtons.length)-buttonSpacing)
					};
				}
			}
		};

		static auto redraw = [](){
			auto width = window->get_width();
			auto height = window->get_height();
			auto &clientArea = window->get_client_area();
			auto &windowArea = window->get_window_area();

			clientArea.draw_rect(0, 0, clientArea.width, clientArea.height, 0xff000000);

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

				auto fontMeasurements = clientArea.measure_text(fontSettings, time, clientArea.width);

				if(clientArea.width>=clientArea.height){
					clientArea.draw_text(fontSettings, time, width-fontMeasurements.blockWidth-12+1, height-fontMeasurements.updatedArea.y2-(height-fontMeasurements.updatedArea.y2+fontMeasurements.updatedArea.y1)/2, fontMeasurements.blockWidth, 0x000000);
					clientArea.draw_text(fontSettings, time, width-fontMeasurements.blockWidth-12, height-fontMeasurements.updatedArea.y2-(height-fontMeasurements.updatedArea.y2+fontMeasurements.updatedArea.y1)/2-1, fontMeasurements.blockWidth, 0xffffff);
				}else{
					clientArea.draw_text(fontSettings, time, (width-fontMeasurements.blockWidth)/2+1, height-fontMeasurements.updatedArea.y2-fontMeasurements.updatedArea.y2-12+1, fontMeasurements.blockWidth, 0x000000);
					clientArea.draw_text(fontSettings, time, (width-fontMeasurements.blockWidth)/2, height-fontMeasurements.updatedArea.y2-fontMeasurements.updatedArea.y2-12, fontMeasurements.blockWidth, 0xffffff);
				}
			}

			gui.buffer = clientArea;
			if(clientArea.width>=clientArea.height){
				launcherButton.rect = {padding, padding, padding+(I32)cleanTheme.get_minimum_button_width(), padding+(I32)clientArea.height-padding-padding};
			}else{
				launcherButton.rect = {padding, padding, padding+(I32)clientArea.width-padding-padding, padding+(I32)(cleanTheme.get_minimum_button_width()*1/3+cleanTheme.get_minimum_button_height()*2/3)};
			}

			layoutWindowButtons();
			gui.redraw();
		};

		{
			auto i=0u;
			for(auto desktopWindow=desktopManager->get_window(i); desktopWindow; desktopWindow=desktopManager->get_window(++i)){
				if(desktopWindow==window) continue;
				windowButtons.push_back(new WindowButton(gui, {0,0,0,0}, *desktopWindow));
			}
		}

		redraw();
		window->redraw();

		desktopManager->events.subscribe([](const driver::DesktopManager::Event &event){
			if(event.type==driver::DesktopManager::Event::Type::windowAdded){
				windowButtons.push_back(new WindowButton(gui, {0,0,0,0}, *event.windowAdded.window));
				redraw();
				window->redraw();

			}else if(event.type==driver::DesktopManager::Event::Type::windowRemoved){
				for(auto i=0u;i<windowButtons.length;i++){
					if(&windowButtons[i]->window==event.windowAdded.window){
						windowButtons.remove(i);
						redraw();
						window->redraw();
						break;
					}
				}

			}else if(event.type==driver::DesktopManager::Event::Type::windowFocused){
				update_windowButton_states();
			}
		});

		window->events.subscribe([](const driver::DesktopManager::Window::Event &event){
			if(event.type==driver::DesktopManager::Window::Event::Type::clientAreaChanged){
				redraw();
				window->redraw();

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
