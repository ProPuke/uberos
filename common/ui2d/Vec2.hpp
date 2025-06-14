#pragma once

#include <common/maths.hpp>
#include <common/Optional.hpp>
#include <common/types.hpp>

template <typename Type>
struct Vec2 {
	Type x, y;
};

template <typename Type>
auto min(Vec2<Type> a, Vec2<Type> b) -> Vec2<Type> { return {maths::min(a.x, b.x), maths::min(a.y, b.y)}; }
template <typename Type>
auto max(Vec2<Type> a, Vec2<Type> b) -> Vec2<Type> { return {maths::max(a.x, b.x), maths::max(a.y, b.y)}; }

typedef Vec2<U32> UVec2;
typedef Vec2<I32> IVec2;
typedef Vec2<Optional<U32>> UVec2Optional;
typedef Vec2<Optional<I32>> IVec2Optional;
