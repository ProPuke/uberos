#pragma once

#include <common/ui2d/Gui.hpp>
#include <common/ui2d/LayoutControl.hpp>

namespace ui2d::controlContainer {
	struct WrapBox: LayoutContainer {
		typedef LayoutContainer Super;

		enum struct Direction {
			vertical,
			horizontal
		};

		I32 spacing = 8;
		bool fillSpace = false;
		Direction direction = Direction::horizontal;
		float alignment = 0.0f;

		/**/ WrapBox(Gui &gui, Direction = Direction::horizontal);
		/**/ WrapBox(Gui &gui, LayoutContainer *container, Direction = Direction::horizontal);

		auto get_min_size() -> IVec2 override;

		void set_spacing(I32);
		void set_direction(Direction);
		void set_alignment(float);

		void layout() override;
	};
}
