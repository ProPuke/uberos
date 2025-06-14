#include "driveList.hpp"

#include <drivers/DesktopManager.hpp>
#include <drivers/StorageController.hpp>
#include <drivers/StorageManager.hpp>

#include <common/ui2d/ApplicationWindow.hpp>
#include <common/ui2d/control/Button.hpp>
#include <common/ui2d/control/Icon.hpp>
#include <common/ui2d/control/Label.hpp>
#include <common/ui2d/controlContainer/Box.hpp>
#include <common/ui2d/controlContainer/IconList.hpp>
#include <common/ui2d/controlContainer/WrapBox.hpp>

#include <kernel/drivers.hpp>

namespace ui2d::image {
	namespace icons::devices {
		extern graphics2d::MultisizeIcon drive_hd;
		extern graphics2d::MultisizeIcon drive_cd;
	}
}

namespace tests::driveList {
	namespace {
		driver::DesktopManager *desktopManager;
		driver::StorageManager *storageManager;
		ui2d::ApplicationWindow *window;
	}

	void run() {
		desktopManager = drivers::find_and_activate<driver::DesktopManager>();
		storageManager = drivers::find_and_activate<driver::StorageManager>();
		if(!storageManager) return;

		window = new ui2d::ApplicationWindow("Drive List", 700, 600);

		window->guiLayout.set_direction(ui2d::controlContainer::Box::Direction::horizontal);
		auto &list = window->guiLayout.add_container_control<ui2d::controlContainer::Box>(ui2d::controlContainer::Box::Direction::vertical);
		list.set_expand(false, true);
		list.set_style(ui2d::controlContainer::Box::Style::inset);

		static ui2d::LayoutControl<ui2d::control::Label> *labelInfo;
		static ui2d::controlContainer::Box *actionBar;
		static ui2d::LayoutControl<ui2d::control::Button> *ejectButton;

		struct IconList: ui2d::controlContainer::IconList {
			typedef ui2d::controlContainer::IconList Super;

			/**/ IconList(ui2d::Gui &gui, LayoutContainer *container):
				Super(gui, container)
			{}

			void on_selected(void *_i) override {
				auto i = (U32)_i;

				static char buffer[1024];

				buffer[0] = '\0';
				strcat(buffer, "Name: ");
				strcat(buffer, TRY_RESULT_OR(storageManager->get_drive_name(i), "???"));
				strcat(buffer, "\nDescription: ");
				strcat(buffer, TRY_RESULT_OR(storageManager->get_drive_description(i), "???"));
				strcat(buffer, "\nModel: ");
				strcat(buffer, TRY_RESULT_OR(storageManager->get_drive_model(i), "???"));
				strcat(buffer, "\nSerial Number: ");
				strcat(buffer, TRY_RESULT_OR(storageManager->get_drive_serialNumber(i), "???"));
				strcat(buffer, "\nDriver: ");
				auto controller = TRY_RESULT_OR(storageManager->get_drive_controller(i), nullptr);
				if(controller){
					strcat(buffer, controller->type->name);
					strcat(buffer, " (");
					strcat(buffer, controller->type->description);
					strcat(buffer, ")");
				}else{
					strcat(buffer, "???");
				}
				strcat(buffer, "\nSize: ");
				auto size = storageManager->get_drive_size(i);
				if(size){
					auto value = size.result;
					if(value>=10*1024){
						if(value>=1024*1024){
							strcat(buffer, to_string(value/(1024*1024.0)));
							strcat(buffer, " MiB");
						}else{
							strcat(buffer, to_string(value/1024.0));
							strcat(buffer, " KiB");
						}

					}else if(value==0){
						strcat(buffer, "0");
					}else{
						strcat(buffer, to_string(value));
						strcat(buffer, " bytes");
					}
				}else{
					strcat(buffer, "???");
				}
				strcat(buffer, "\nPresent: ");
				strcat(buffer, TRY_RESULT_OR(storageManager->is_drive_present(i), false)?"Yes":"No");
				strcat(buffer, "\nRemovable: ");
				const auto isRemovable = TRY_RESULT_OR(storageManager->is_drive_removable(i), false);
				strcat(buffer, isRemovable?"Yes":"No");

				labelInfo->text = buffer;

				labelInfo->container->_on_children_changed();
				labelInfo->redraw();

				if(ejectButton->isVisible!=isRemovable){
					actionBar->isVisible = isRemovable;
					ejectButton->isVisible = isRemovable;
					window->layout();
				}
			}
		};

		auto &iconList = list.add_container_control<IconList>();

		// list.add_control<ui2d::control::Icon>(ui2d::image::icons::devices::drive_hd, "hd").set_selected(true);
		// list.add_control<ui2d::control::Icon>(ui2d::image::icons::devices::drive_cd, "cd");
		auto &main = window->guiLayout.add_container_control<ui2d::controlContainer::Box>(ui2d::controlContainer::Box::Direction::vertical);
		auto &details = main.add_container_control<ui2d::controlContainer::Box>(ui2d::controlContainer::Box::Direction::vertical);
		details.set_style(ui2d::controlContainer::Box::Style::inset);

		labelInfo = &details.add_control<ui2d::control::Label>(""); labelInfo->set_expand(true, false);

		actionBar = &main.add_container_control<ui2d::controlContainer::Box>(ui2d::controlContainer::Box::Direction::horizontal);
		actionBar->set_expand(true, false);
		actionBar->add_container_control<ui2d::controlContainer::Box>();
		ejectButton = &actionBar->add_control<ui2d::control::Button>("Eject");
		ejectButton->set_fixed_size(80, 26);

		for(auto i=0u;i<storageManager->get_drive_count();i++){
			if(!storageManager->does_drive_exist(i)) continue;

			auto name = TRY_RESULT_OR(storageManager->get_drive_name(i), "???");
			auto removeable = TRY_RESULT_OR(storageManager->is_drive_removable(i), false);

			iconList.add_icon((void*)i, removeable?ui2d::image::icons::devices::drive_cd:ui2d::image::icons::devices::drive_hd, name);
		}
		// iconList.add_icon(ui2d::image::icons::devices::drive_hd, "hd");
		// iconList.add_icon(ui2d::image::icons::devices::drive_cd, "cd");

		iconList.select(0);

		// get_drive_name
		// get_drive_size
		// get_drive_model
		// get_drive_serialNumber
		// get_drive_systemId
		// is_drive_present
		// is_drive_removable
		// is_system_drive

		window->redraw();
		window->show();
	}
}
