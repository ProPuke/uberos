#pragma once

#include "types.hpp"

extern "C" void memcpy(void *dest, const void *src, unsigned bytes);
extern "C" void memcpy_forwards(void *dest, const void *src, unsigned bytes);
extern "C" void memcpy_backwards(void *dest, const void *src, unsigned bytes);
extern "C" void memmove(void *dest, const void *src, unsigned bytes);
extern "C" int memcmp(const void *a, const void *b, unsigned length);
extern "C" void bzero(void *dest, unsigned bytes);
extern "C" unsigned strlen(const C8 *str);
extern "C" char* strcat(char *destination, const char *source);
extern "C" int strcmp(const char* str1, const char* str2);

constexpr inline U32 bits(U32 data, U8 start, U8 end){ return (data&((((U32)1<<(end-start+1)) - 1) << start)) >> start; }
constexpr inline U64 bits(U64 data, U8 start, U8 end){ return (data&((((U64)1<<(end-start+1)) - 1) << start)) >> start; }

constexpr inline U64 bitmask(U8 min, U8 max) { return ((1ll<<(max-min+1))-1)<<min; }

constexpr inline U8 bit_count(U64 i) { U8 bits=0; while(i){ i&=i-1; ++bits; } return bits; }
constexpr inline U64 bit_rightmost(U64 i) { return i^(i&(i-1)); }
constexpr inline U8 bit_rightmost_position(U64 i) { return i==0?0:bit_count(bit_rightmost(i)-1); }

inline U32 sign_extend(U32 number, U8 numbits){ return number&(1<<(numbits-1))?number|~((1<<numbits)-1):number; }
inline U64 sign_extend(U64 number, U8 numbits){ return number&(1<<(numbits-1))?number|~((1<<numbits)-1):number; }

template <typename T, typename... T2> T min(T x) { return x; }
template <typename T, typename... T2> T min(T x, T y, T2...others) { return min(x< y?x:y, others...); }

template <typename T, typename... T2> T max(T x) { return x; }
template <typename T, typename... T2> T max(T x, T y, T2...others) { return max(x>=y?x:y, others...); }

auto to_string(U16 i) -> const char*;
auto to_string(I16 i) -> const char*;
auto to_string(U32 i) -> const char*;
auto to_string(I32 i) -> const char*;
auto to_string(U64 i) -> const char*;
auto to_string(I64 i) -> const char*;

auto to_string_hex(U8 i) -> const char*;
auto to_string_hex(U16 i) -> const char*;
auto to_string_hex(U32 i) -> const char*;
auto to_string_hex(U64 i) -> const char*;
auto to_string_hex_trim(U8 i) -> const char*;
auto to_string_hex_trim(U16 i) -> const char*;
auto to_string_hex_trim(U32 i) -> const char*;
auto to_string_hex_trim(U64 i) -> const char*;

#if defined(_32BIT)
	inline auto to_string(void *pointer) -> const char* { return to_string_hex((U32)pointer); }
#elif defined(_64BIT)
	inline auto to_string(void *pointer) -> const char* { return to_string_hex((U64)pointer); }
#endif

//NOTE:HACK: int32_t and uint32_t aren't typedefs of int and unsigned int literals with this compiler, even though it's 32bit
#if defined(ARCH_ARM32)
	inline auto to_string(int i) -> const char* { return to_string((I32)i); }
	inline auto to_string(unsigned int i) -> const char* { return to_string((U32)i); }
	inline auto to_string_hex(unsigned int i) -> const char* { return to_string_hex((U32)i); }
	inline auto to_string_hex_trim(unsigned int i) -> const char* { return to_string_hex_trim((U32)i); }
#endif
