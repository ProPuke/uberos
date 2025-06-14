#pragma once

#include "../Control.hpp"

#include <common/Optional.hpp>

namespace graphics2d {
	struct Buffer;
}

namespace ui2d {
	namespace control {
		struct Label: Control {
			typedef Control Super;

			/**/ Label(Gui &gui, graphics2d::Rect rect, const char *text):
				Super(gui, rect),
				text(text)
			{}

			const char *text;
			U32 fontSize = 14;
			Optional<U32> colour;

			void set_text(const char*);
			void set_fontSize(U32);
			void set_colour(U32);

			auto get_min_size() -> IVec2 override;
			auto get_max_size() -> IVec2 override;

			void redraw(bool flush = true) override;
		};
	}
}
