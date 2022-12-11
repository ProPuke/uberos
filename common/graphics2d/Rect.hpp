#pragma once

#include <common/types.hpp>
#include <common/stdlib.hpp>

namespace graphics2d {

	struct Rect {
		I32 x1, y1, x2, y2;

		void offset(I32 x, I32 y) {
			x1 += x;
			x2 += x;
			y1 += y;
			y2 += y;
		}

		void include(Rect with){
			x1 = min(x1, with.x1);
			x2 = max(x2, with.x2);
			y1 = min(y1, with.y1);
			y2 = max(y2, with.y2);
		}

		bool isNonzero() const { return x1||y1||x2||y2; }
	};

}
