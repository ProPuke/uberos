#pragma once

#include <common/ui2d/control/Icon.hpp>
#include <common/ui2d/controlContainer/WrapBox.hpp>

namespace ui2d::controlContainer {
	struct IconList: WrapBox {
		typedef WrapBox Super;

		/**/ IconList(Gui &gui);
		/**/ IconList(Gui &gui, LayoutContainer *container);

		auto add_icon(void *data, graphics2d::MultisizeIcon icon, const char *label) -> control::Icon&;
		void remove_control(LayoutControlBase&) override;

		void select(void *data);

		virtual void on_selected(void *data){}
	};
}
