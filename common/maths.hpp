#pragma once

#include "types.hpp"

U16 rand();

template <typename Type> inline Type min(Type x, Type y) { return x<=y?x:y; }
template <typename Type> inline Type max(Type x, Type y) { return x>=y?x:y; }
template <typename Type> inline Type abs(Type x) { return x>=0?x:-x; }
template <typename InputType, typename OutputType> inline OutputType clamp(InputType x, OutputType a, OutputType b) { return min<InputType>(max<InputType>(x, a), b); }
