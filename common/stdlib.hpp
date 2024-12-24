#pragma once

#include "types.hpp"

#include "stdlib.h"

extern "C" auto memcpy(void *__restrict dest, const void *__restrict src, size_t bytes) -> void*;
extern "C" auto memcpy_aligned(void *__restrict dest, const void *__restrict src, size_t bytes) -> void*;
extern "C" auto memcpy_forwards_aligned(void *__restrict dest, const void *__restrict src, size_t bytes) -> void*;
extern "C" auto memcpy_backwards_aligned(void *__restrict dest, const void *__restrict src, size_t bytes) -> void*;
extern "C" auto memmove(void *dest, const void *src, size_t bytes) -> void*;
extern "C" auto memcmp(const void *a, const void *b, size_t bytes) -> int;
extern "C" auto memset(void *dest, int value, size_t bytes) -> void*;

extern "C" auto bzero(void *dest, size_t bytes) -> void;

extern "C" auto strlen(const C8 *str) -> size_t;
extern "C" auto strcpy(char *__restrict destination, const char *__restrict source) -> char*;
extern "C" auto strcat(char *destination, const char *source) -> char*;
extern "C" auto strcmp(const char* str1, const char* str2) -> int;

template <typename T> constexpr inline auto align(/* */ T *pointer, U8 alignment) -> /* */ T* { return (/* */ T*)((size_t)pointer+((size_t)pointer%alignment?(alignment-(size_t)pointer%alignment):0)); }
template <typename T> constexpr inline auto align(const T *pointer, U8 alignment) -> const T* { return (const T*)((size_t)pointer+((size_t)pointer%alignment?(alignment-(size_t)pointer%alignment):0)); }
template <typename T> constexpr inline auto align(T x, U8 alignment) -> T { return x+(x%alignment?(alignment-x%alignment):0); }

constexpr inline auto bits(U8  data, U8 start, U8 end) -> U8  { return (data&((((U8 )1<<(end-start+1)) - 1) << start)) >> start; }
constexpr inline auto bits(U16 data, U8 start, U8 end) -> U16 { return (data&((((U16)1<<(end-start+1)) - 1) << start)) >> start; }
constexpr inline auto bits(U32 data, U8 start, U8 end) -> U32 { return (data&((((U32)1<<(end-start+1)) - 1) << start)) >> start; }
constexpr inline auto bits(U64 data, U8 start, U8 end) -> U64 { return (data&((((U64)1<<(end-start+1)) - 1) << start)) >> start; }

constexpr inline auto bitmask(U8 min, U8 max) -> U64 { return ((1ll<<(max-min+1))-1)<<min; }

constexpr inline auto bit_count(U64 i) -> U8 { U8 bits=0; while(i){ i&=i-1; ++bits; } return bits; }
constexpr inline auto bit_rightmost(U64 i) -> U64 { return i^(i&(i-1)); }
constexpr inline auto bit_rightmost_position(U64 i) -> U8 { return i==0?0:bit_count(bit_rightmost(i)-1); }

constexpr inline auto sign_extend(U32 number, U8 numbits) -> U32 { return number&(1<<(numbits-1))?number|~((1<<numbits)-1):number; }
constexpr inline auto sign_extend(U64 number, U8 numbits) -> U64 { return number&(1<<(numbits-1))?number|~((1<<numbits)-1):number; }

template <typename T, typename... T2> T min(T x) { return x; }
template <typename T, typename... T2> T min(T x, T y, T2...others) { return min(x< y?x:y, others...); }

template <typename T, typename... T2> T max(T x) { return x; }
template <typename T, typename... T2> T max(T x, T y, T2...others) { return max(x>=y?x:y, others...); }

auto utoa(U16) -> const char*;
auto itoa(I16) -> const char*;
auto utoa(U32) -> const char*;
auto itoa(I32) -> const char*;
auto utoa(U64) -> const char*;
auto itoa(I64) -> const char*;
auto ftoa(F32) -> const char*;
auto ftoa(F64) -> const char*;

template<typename Type>
inline auto to_string(Type x) -> const char* { return x.to_string(); }

template<> inline auto to_string(const char* x) -> const char* { return x; }

template<> inline auto to_string(U8 x) -> const char* { return utoa((U16)x); }
template<> inline auto to_string(I8 x) -> const char* { return itoa((I16)x); }
template<> inline auto to_string(U16 x) -> const char* { return utoa(x); }
template<> inline auto to_string(I16 x) -> const char* { return itoa(x); }
template<> inline auto to_string(U32 x) -> const char* { return utoa(x); }
template<> inline auto to_string(I32 x) -> const char* { return itoa(x); }
template<> inline auto to_string(U64 x) -> const char* { return utoa(x); }
template<> inline auto to_string(I64 x) -> const char* { return itoa(x); }
template<> inline auto to_string(F32 x) -> const char* { return ftoa(x); }
template<> inline auto to_string(F64 x) -> const char* { return ftoa(x); }
template<> inline auto to_string(size_t x) -> const char* { return sizeof(x)==sizeof(U32)?to_string((U32)x):to_string((U64)x); }

auto to_string_hex(U8 i) -> const char*;
auto to_string_hex(U16 i) -> const char*;
auto to_string_hex(U32 i) -> const char*;
auto to_string_hex(U64 i) -> const char*;
auto to_string_hex_trim(U8 i) -> const char*;
auto to_string_hex_trim(U16 i) -> const char*;
auto to_string_hex_trim(U32 i) -> const char*;
auto to_string_hex_trim(U64 i) -> const char*;
inline auto to_string_hex_trim(size_t x) -> const char* { return sizeof(x)==sizeof(U32)?to_string_hex_trim((U32)x):to_string_hex_trim((U64)x); }

#include "maths.hpp"

template <bool align>
void* __attribute__ ((optimize(2))) memory_copy_forwards(void *__restrict dest, const void *__restrict src, size_t size) {
	if(align){
		auto align_dest = maths::min(size, (size_t)dest%8);
		auto align_src = maths::min(size, (size_t)src%8);

		if(align_dest!=align_src){
			while(size--) *(*(U8**)&dest)++ = *(*(U8**)&src)++;
			return dest;
		}

		if(align_dest){
			align_dest = 8-align_dest;

			switch(8-align_dest){
				case 7:
					*(*(U8**)&dest)++ = *(*(U8**)&src)++;
				case 6:
					*(*(U8**)&dest)++ = *(*(U8**)&src)++;
				case 5:
					*(*(U8**)&dest)++ = *(*(U8**)&src)++;
				case 4:
					*(*(U32**)&dest)++ = *(*(U32**)&src)++;
				break;
				case 3:
					*(*(U8**)&dest)++ = *(*(U8**)&src)++;
				case 2:
					*(*(U8**)&dest)++ = *(*(U8**)&src)++;
				case 1:
					*(*(U8**)&dest)++ = *(*(U8**)&src)++;
			}

			size -= align_dest;
		}
	}

	{
		size_t _8s = size/8;
		size -= _8s*8;

		while(_8s--) *(*(U64**)&dest)++ = *(*(U64**)&src)++;
	}

	if(size>=4){
		*(*(U32**)&dest)++ = *(*(U32**)&src)++;
		size -= 4;
	}

	switch(size){
		case 3:
			*(*(U8**)&dest)++ = *(*(U8**)&src)++;
		case 2:
			*(*(U8**)&dest)++ = *(*(U8**)&src)++;
		case 1:
			*(*(U8**)&dest)++ = *(*(U8**)&src)++;
	}

	return dest;
}

// inline void* memcpy_forwards_aligned(void *__restrict dest, const void *__restrict src, size_t bytes) {
// 	return memory_copy_forwards<true>(dest, src, bytes);
// }

#if defined(_32BIT)
	template<> inline auto to_string(const void *pointer) -> const char* { return to_string_hex((U32)pointer); }
	template<> inline auto to_string(/* */ void *pointer) -> const char* { return to_string_hex((U32)pointer); }
#elif defined(_64BIT)
	template<> inline auto to_string(const void *pointer) -> const char* { return to_string_hex((U64)pointer); }
	template<> inline auto to_string(/* */ void *pointer) -> const char* { return to_string_hex((U64)pointer); }
#endif

//NOTE:HACK: int32_t and uint32_t aren't typedefs of int and unsigned int literals with this compiler, even though it's 32bit
#if defined(ARCH_ARM32)
	template<> inline auto to_string(int i) -> const char* { return to_string((I32)i); }
	template<> inline auto to_string(unsigned int i) -> const char* { return to_string((U32)i); }
	inline auto to_string_hex(unsigned int i) -> const char* { return to_string_hex((U32)i); }
	inline auto to_string_hex_trim(unsigned int i) -> const char* { return to_string_hex_trim((U32)i); }
#endif

#if defined(ARCH_ARM64)
	#include "arch/arm64/stdlib.hpp"
#endif
