#pragma once

#include <common/types.hpp>

namespace graphics2d {
	// inline void create_round_corner(U32 radius, U32 corner[]) {
	// 	for(auto i=0u;i<radius;i++){
	// 		auto y = radius-i;
	// 		corner[i] = radius-U32(radius*maths::sqrt(radius*radius-((y*y))));
	// 	}
	// 	corner[radius] = -1;
	// }

	inline void create_diagonal_corner(U32 radius, U32 corner[]) {
		for(auto i=0u;i<radius;i++){
			corner[i] = radius-i;
		}
		corner[radius] = -1;
	}

	inline auto premultiply_colour(U32 colour) -> U32 {
		const auto alpha = 255-((colour&0xff000000)>>24);
		return
			((colour>> 0&0xff)*alpha/255)<< 0|
			((colour>> 8&0xff)*alpha/255)<< 8|
			((colour>>16&0xff)*alpha/255)<<16|
			colour&0xff000000
		;
	}
}
