#pragma once

#include <common/types.hpp>
#include <common/stdlib.hpp>

namespace format {
	struct Hex8  { U8  value; bool head = true; };
	struct Hex16 { U16 value; bool head = true; };
	struct Hex32 {
		U32 value;
		bool head = true;

		#ifdef _32BIT
			Hex32(U32 value):value(value){}
			Hex32(void *pointer):value((U32)pointer){}
		#endif
	};
	struct Hex64 {
		U64 value;
		bool head = true;

		#ifdef _64BIT
			Hex64(U64 value):value(value){}
			Hex64(void *pointer):value((U64)pointer){}
		#endif
	};
}

inline auto to_string(format::Hex8  x) -> const char* { return to_string_hex(x.value)+(x.head?0:2); }
inline auto to_string(format::Hex16 x) -> const char* { return to_string_hex(x.value)+(x.head?0:2); }
inline auto to_string(format::Hex32 x) -> const char* { return to_string_hex(x.value)+(x.head?0:2); }
inline auto to_string(format::Hex64 x) -> const char* { return to_string_hex(x.value)+(x.head?0:2); }
