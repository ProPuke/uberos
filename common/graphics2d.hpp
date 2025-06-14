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

	inline auto blend_colours(U32 colourA, U32 colourB) -> U32 {
		const auto trans = colourB>>24;
		return
			(((colourA>> 0&0xff)*trans/255)+(colourB>> 0&0xff))<< 0|
			(((colourA>> 8&0xff)*trans/255)+(colourB>> 8&0xff))<< 8|
			(((colourA>>16&0xff)*trans/255)+(colourB>>16&0xff))<<16|
			((colourA>>24)*trans/255)<<24
		;
	}

	inline auto apply_colour_difference(U32 colour, U32 from, U32 to) -> U32 {
		// apply the difference from `from` -> `to` to `colour` (colour * to/from)
		return
			(U8)maths::min(0xffu, (colour>> 0&0xff)*(to>> 0&0xff)/(from>> 0&0xff))<< 0|
			(U8)maths::min(0xffu, (colour>> 8&0xff)*(to>> 8&0xff)/(from>> 8&0xff))<< 8|
			(U8)maths::min(0xffu, (colour>>16&0xff)*(to>>16&0xff)/(from>>16&0xff))<<16|
			(U8)maths::min(0xffu, 255-(255-colour>>24&0xff)*(255-to>>24&0xff)/maths::max(1u, 255-from>>24&0xff))<<24
		;
	}

	inline auto multiply_colours(U32 colourA, U32 colourB) -> U32 {
		const auto alphaB = 255-(colourB>>24);
		return
			(((colourA>> 0&0xff)*alphaB/255)*(colourB>> 0&0xff)/255)<< 0|
			(((colourA>> 8&0xff)*alphaB/255)*(colourB>> 8&0xff)/255)<< 8|
			(((colourA>>16&0xff)*alphaB/255)*(colourB>>16&0xff)/255)<<16|
			(255-(255-(colourA>>24))*alphaB/255)<<24
		;
	}

	inline auto blend_colours(U32 colourA, U32 colourB, U8 opacity) -> U32 {
		const auto trans = 255-((255-colourB>>24)*opacity/255);
		return
			(((colourA>> 0&0xff)*trans/255)+(colourB>> 0&0xff)*opacity/255)<< 0|
			(((colourA>> 8&0xff)*trans/255)+(colourB>> 8&0xff)*opacity/255)<< 8|
			(((colourA>>16&0xff)*trans/255)+(colourB>>16&0xff)*opacity/255)<<16|
			((colourA>>24)*trans/255)<<24
		;
	}
}
