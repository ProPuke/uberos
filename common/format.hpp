#pragma once

#include <common/types.hpp>
#include <common/stdlib.hpp>

namespace format {
	struct Hex8  { U8  value; bool head = true; bool full = true; };
	struct Hex16 { U16 value; bool head = true; bool full = true; };
	struct Hex32 {
		U32 value;
		bool head = true;
		bool full = true;

		#ifdef _32BIT
			Hex32(U32 value, bool head = true, bool full = true):value(value), head(head), full(full){}
			Hex32(volatile void *pointer):value((U32)pointer){}
		#endif
	};
	struct Hex64 {
		U64 value;
		bool head = true;
		bool full = true;

		#ifdef _64BIT
			Hex64(U64 value, bool head = true, bool full = true):value(value), head(head), full(full){}
			Hex64(volatile void *pointer):value((U64)pointer){}
		#endif
	};
}

#include <common/stdlib.hpp>

template<> inline auto to_string(format::Hex8  x) -> const char* { return (x.full?to_string_hex(x.value):to_string_hex_trim(x.value))+(x.head?0:2); }
template<> inline auto to_string(format::Hex16 x) -> const char* { return (x.full?to_string_hex(x.value):to_string_hex_trim(x.value))+(x.head?0:2); }
template<> inline auto to_string(format::Hex32 x) -> const char* { return (x.full?to_string_hex(x.value):to_string_hex_trim(x.value))+(x.head?0:2); }
template<> inline auto to_string(format::Hex64 x) -> const char* { return (x.full?to_string_hex(x.value):to_string_hex_trim(x.value))+(x.head?0:2); }

template<typename Type>
inline auto to_string(const Type *x) -> const char* { return to_string(format::Hex64{x}); }
template<typename Type>
inline auto to_string(Type *x) -> const char* { return to_string(format::Hex64{x}); }
