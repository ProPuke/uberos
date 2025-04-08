#pragma once

#include <common/ui2d/Gui.hpp>
#include <common/ui2d/LayoutControl.hpp>

namespace ui2d::controlContainer {
	struct Box: LayoutContainer {
		typedef LayoutContainer Super;

		enum struct Direction {
			vertical,
			horizontal
		};

		U32 spacing = 8;
		Direction direction = Direction::horizontal;
		float alignment = 0.0f;

		/**/ Box(Gui &gui);
		/**/ Box(Gui &gui, LayoutContainer *container);

		auto get_min_size() -> UVec2 override;
		auto get_max_size() -> UVec2 override;
		void set_spacing(U32);
		void set_direction(Direction);
		void set_alignment(float);

		void layout() override;
	};
}
