#pragma once

template <typename Type, Type min, Type max>
class Range {
	Type value;

public:

	constexpr /**/ Range(Type value = 0):value(value) { static_assert(value>=min&&value<=max); }
	/**/ Range(Type value):value(value) { _debug_assert(); }

	void _debug_assert(){
		debug::assert(value>=min&&value<=max);
	}

	constexpr auto operator=(Type x) { value = x; static_assert(value>=min&&value<=max); return *this; }
	auto operator=(Type x) { value = x; _debug_assert(); return *this; }
	auto operator+=(Type x) { value += x; _debug_assert(); return *this; }
	auto operator-=(Type x) { value -= x; _debug_assert(); return *this; }
	auto operator*=(Type x) { value *= x; _debug_assert(); return *this; }
	auto operator/=(Type x) { value /= x; _debug_assert(); return *this; }
	auto operator%=(Type x) { value %= x; _debug_assert(); return *this; }
	auto operator<<=(Type x) { value <<= x; _debug_assert(); return *this; }
	auto operator>>=(Type x) { value >>= x; _debug_assert(); return *this; }
	auto operator&=(Type x) { value &= x; _debug_assert(); return *this; }
	auto operator|=(Type x) { value |= x; _debug_assert(); return *this; }
	auto operator^=(Type x) { value ^= x; _debug_assert(); return *this; }

	// template <Type min, Type max> auto operator+=(Range<Type, min, max> x) { value += x.value; _debug_assert(); return *this; }
	// template <Type min, Type max> auto operator-=(Range<Type, min, max> x) { value -= x.value; _debug_assert(); return *this; }
	// template <Type min, Type max> auto operator*=(Range<Type, min, max> x) { value *= x.value; _debug_assert(); return *this; }
	// template <Type min, Type max> auto operator/=(Range<Type, min, max> x) { value /= x.value; _debug_assert(); return *this; }
	// template <Type min, Type max> auto operator%=(Range<Type, min, max> x) { value %= x.value; _debug_assert(); return *this; }
	// template <Type min, Type max> auto operator<<=(Range<Type, min, max> x) { value <<= x.value; _debug_assert(); return *this; }
	// template <Type min, Type max> auto operator>>=(Range<Type, min, max> x) { value >>= x.value; _debug_assert(); return *this; }
	// template <Type min, Type max> auto operator&=(Range<Type, min, max> x) { value &= x.value; _debug_assert(); return *this; }
	// template <Type min, Type max> auto operator|=(Range<Type, min, max> x) { value |= x.value; _debug_assert(); return *this; }
	// template <Type min, Type max> auto operator^=(Range<Type, min, max> x) { value ^= x.value; _debug_assert(); return *this; }

	operator Type() const { return value; }
};
