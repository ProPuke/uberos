#pragma once

namespace graphics2d {

	struct Rect {
		I32 x1, y1, x2, y2;

		void offset(I32 x, I32 y) {
			x1 += x;
			x2 += x;
			y1 += y;
			y2 += y;
		}
	};

}
