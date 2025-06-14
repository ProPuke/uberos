#pragma once

#include "../Control.hpp"

namespace graphics2d {
	struct Buffer;
}

namespace ui2d {
	namespace control {
		struct Area: Control {
			typedef Control Super;

			/**/ Area(Gui &gui, graphics2d::Rect rect):
				Super(gui, rect)
			{}

			IVec2 minSize{0,0};
			IVec2 maxSize{0,0};
			auto get_min_size() -> IVec2 override { return minSize; }
			auto get_max_size() -> IVec2 override { return maxSize; }
		};
	}
}
