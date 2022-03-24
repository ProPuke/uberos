#pragma once

#include "types.hpp"

U16 rand();

template <typename Type>
inline Type min(Type x, Type y) { return x<=y?x:y; }
template <typename Type>
inline Type max(Type x, Type y) { return x>=y?x:y; }

inline U8  abs(I8  x) { return x>=0?x:-x; }
inline U16 abs(I16 x) { return x>=0?x:-x; }
inline U32 abs(I32 x) { return x>=0?x:-x; }
inline U64 abs(I64 x) { return x>=0?x:-x; }

template <typename InputType, typename OutputType>
inline OutputType clamp(InputType x, OutputType a, OutputType b) { return min<InputType>(max<InputType>(x, a), b); }

template <typename T>
inline auto celcius_to_kelvin(T x) { return x+273.15; }
template <typename T>
inline auto kelvin_to_celcius(T x) { return x-273.15; }
