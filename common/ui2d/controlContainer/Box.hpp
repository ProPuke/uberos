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

		enum struct Style {
			none,
			padded,
			border,
			inset
		};

		I32 spacing = 8;
		Direction direction = Direction::horizontal;
		Style style = Style::none;
		float alignment = 0.0f;

		/**/ Box(Gui &gui, Direction = Direction::horizontal);
		/**/ Box(Gui &gui, LayoutContainer *container, Direction = Direction::horizontal);

		auto get_min_size() -> IVec2 override;
		auto get_max_size() -> IVec2 override;
		void set_spacing(I32);
		void set_direction(Direction);
		void set_style(Style);
		void set_alignment(float);

		void layout() override;
		void redraw(bool flush) override;
	};
}
