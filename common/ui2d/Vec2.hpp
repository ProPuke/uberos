#pragma once

#include <common/types.hpp>

template <typename Type>
struct Vec2 {
	Type x, y;
};

typedef Vec2<U32> UVec2;
typedef Vec2<I32> IVec2;
