#pragma once

#include "graphics2d.hpp"

namespace graphics2d {
	inline void _update_view(View &view) { return _update_view_area(view, {0, 0, (I32)view.buffer.width*(I32)view.scale, (I32)view.buffer.height*(I32)view.scale}); };
}
