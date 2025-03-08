#pragma once

#include "Button.hpp"

namespace ui2d {
	namespace control {
		struct ColouredButton: Button {
			/**/ ColouredButton(Gui&, graphics2d::Rect, U32 colour, const char *text);

			U32 colour;
			U8 opacity = 0xff;

			void redraw(bool flush = true) override;
		};
	}
}
