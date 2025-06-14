#pragma once

#include "../Control.hpp"

#include <common/graphics2d/MultisizeIcon.hpp>

namespace ui2d {
	namespace control {
		struct Icon: Control {
			typedef Control Super;

			/**/ Icon(Gui &gui, graphics2d::Rect rect, graphics2d::MultisizeIcon icon, const char *label):
				Super(gui, rect),
				icon(icon),
				label(label)
			{}

			graphics2d::MultisizeIcon icon;
			I32 iconSize = 48;
			bool isSelected = false;
			const char *label;

			void set_selected(bool);
			void set_icon_size(I32);

			auto get_min_size() -> IVec2 override;
			auto get_max_size() -> IVec2 override;

			void on_mouse_pressed(I32 x, I32 y, U32 button) override;
			void on_mouse_released(I32 x, I32 y, U32 button) override;

			virtual void on_pressed(){};
			virtual void on_released(){};
			virtual void on_clicked(){};

			void redraw(bool flush = true) override;
		};
	}
}
