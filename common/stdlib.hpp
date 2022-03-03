#pragma once

#include "types.hpp"

extern "C" void memcpy(void *dest, void *src, unsigned bytes);
extern "C" void memcpy_forwards(void *dest, void *src, unsigned bytes);
extern "C" void memcpy_backwards(void *dest, void *src, unsigned bytes);
extern "C" void memmove(void *dest, void *src, unsigned bytes);
extern "C" int memcmp(const void *a, const void *b, unsigned length);
extern "C" void bzero(void *dest, unsigned bytes);
extern "C" unsigned strlen(const C8 *str);

constexpr inline U32 bits(U32 data, U8 start, U8 end){ return (data&((1<<(end-start+1)) - 1) << start) >> start; }
constexpr inline U64 bits(U64 data, U8 start, U8 end){ return (data&((1<<(end-start+1)) - 1) << start) >> start; }

constexpr inline U64 bitmask(U8 min, U8 max) { return ((1ll<<(max-min+1))-1)<<min; }

constexpr inline U8 bit_count(U64 i) { U8 bits=0; while(i){ i&=i-1; ++bits; } return bits; }

inline U32 sign_extend(U32 number, U8 numbits){ return number&(1<<(numbits-1))?number|~((1<<numbits)-1):number; }
inline U64 sign_extend(U64 number, U8 numbits){ return number&(1<<(numbits-1))?number|~((1<<numbits)-1):number; }

const char* to_string(U16 i);
const char* to_string(I16 i);
const char* to_string(U32 i);
const char* to_string(I32 i);
const char* to_string(U64 i);
const char* to_string(I64 i);
const char* to_string(void* i);

const char* to_string_hex(U8 i);
const char* to_string_hex(U16 i);
const char* to_string_hex(U32 i);
const char* to_string_hex(U64 i);
const char* to_string_hex_trim(U8 i);
const char* to_string_hex_trim(U16 i);
const char* to_string_hex_trim(U32 i);
const char* to_string_hex_trim(U64 i);

//NOTE:HACK: int32_t and uint32_t aren't typedefs of int and unsigned int literals with this compiler, even though it's 32bit
#if defined(ARCH_ARM32)
	inline const char* to_string(int i) { return to_string((I32)i); }
	inline const char* to_string(unsigned int i) { return to_string((U32)i); }
	inline const char* to_string_hex(unsigned int i) { return to_string_hex((U32)i); }
	inline const char* to_string_hex_trim(unsigned int i) { return to_string_hex_trim((U32)i); }
#endif
