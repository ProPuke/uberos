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
		};
	}
}
