#pragma once

#include "ColouredButton.hpp"

namespace ui2d {
	namespace control {
		struct TitlebarButton: ColouredButton {
			/**/ TitlebarButton(Gui&, graphics2d::Rect, U32 colour, const char *text);

			bool focused = true;

			void redraw(bool flush = true) override;
		};
	}
}
