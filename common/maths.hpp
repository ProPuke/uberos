#pragma once

#include <common/types.hpp>

namespace maths {
	auto rand() -> U16;

	constexpr F64 pi = 3.14159265358979323846;
	constexpr F64 tau = pi*2;

	extern "C" F64 cos(F64);
	extern "C" F64 acos(F64);
	extern "C" F64 sin(F64);
	extern "C" F64 asin(F64);
	extern "C" F64 tan(F64);
	extern "C" F64 atan(F64);
	extern "C" F32 cosf(F32);
	extern "C" F32 acosf(F32);
	extern "C" F32 sinf(F32);
	extern "C" F32 asinf(F32);
	extern "C" F32 tanf(F32);
	extern "C" F32 atanf(F32);
	extern "C" F64 sqrt(F64);
	extern "C" F32 sqrtf(F32);

	template <typename Type>
	constexpr auto min(Type x, Type y) -> Type { return x<=y?x:y; }
	template <typename Type>
	constexpr auto max(Type x, Type y) -> Type { return x>=y?x:y; }

	template <typename Type>
	constexpr auto abs(Type x) -> Type { return x>=0?x:-x; }

	constexpr auto abs(I8  x) -> U8  { return x>=0?x:-x; }
	constexpr auto abs(I16 x) -> U16 { return x>=0?x:-x; }
	constexpr auto abs(I32 x) -> U32 { return x>=0?x:-x; }
	constexpr auto abs(I64 x) -> U64 { return x>=0?x:-x; }

	template <typename Type>
	constexpr auto sign(Type x) -> I8Fast { return x>0?+1:x<0?-1:0; }

	template <typename Type>
	auto sqrt(Type x) -> Type {
		Type result = x;
		for(Type y = (result+1)/2; y<result; y=(result+x/result)/2) {
			result = y;
		}
		return result;
	}

	template <typename InputType, typename OutputType>
	constexpr auto clamp(InputType x, OutputType a, OutputType b) -> OutputType { return min<InputType>(max<InputType>(x, a), b); }

	template <typename T>
	constexpr auto celcius_to_kelvin(T x) -> T { return x+273.15; }
	template <typename T>
	constexpr auto kelvin_to_celcius(T x) -> T { return x-273.15; }
}
