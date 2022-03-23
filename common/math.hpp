#pragma once

template <typename T>
inline auto celcius_to_kelvin(T x) { return x+273.15; }
template <typename T>
inline auto kelvin_to_celcius(T x) { return x-273.15; }
