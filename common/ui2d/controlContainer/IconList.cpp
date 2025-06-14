#include "IconList.hpp"

namespace ui2d::controlContainer {
	namespace {
		struct Icon: control::Icon {
			typedef control::Icon Super;

			IconList &iconList;
			void *data;

			/**/ Icon(Gui &gui, graphics2d::Rect rect, IconList &iconList, void *data, graphics2d::MultisizeIcon icon, const char *label):
				Super(gui, rect, icon, label),
				iconList(iconList),
				data(data)
			{}

			void on_pressed() override {
				if(isSelected) return;

				isSelected = true;
				for(auto i=0u;i<iconList.children.length;i++){
					auto &icon = *(LayoutControl<Icon>*)iconList.children[i];
					if(&icon==this) continue;
					if(!icon.isSelected) continue;

					icon.isSelected = false;
					icon.redraw();
				}

				iconList.on_selected(data);
			}
		};
	}

	/**/ IconList::IconList(Gui &gui):
		Super(gui)
	{}
	/**/ IconList::IconList(Gui &gui, LayoutContainer *container):
		Super(gui, container)
	{}

	auto IconList::add_icon(void *data, graphics2d::MultisizeIcon icon, const char *label) -> control::Icon& {
		return Super::add_control<Icon>(*this, data, icon, label);
	}

	void IconList::remove_control(LayoutControlBase &control) {
		Super::remove_control(control);
	}

	void IconList::select(void *data) {
		for(auto i=0u;i<children.length;i++){
			auto &icon = *(LayoutControl<Icon>*)children[i];

			auto state = icon.data == data;

			if(icon.isSelected==state) continue;

			icon.isSelected = state;
			icon.redraw();
		}

		on_selected(data);
	}
}
