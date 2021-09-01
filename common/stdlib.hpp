#pragma once

#include "types.hpp"

void memcpy(void *dest, void *src, int bytes);
void bzero(void *dest, int bytes);

const char* to_string(U16 i);
const char* to_string(I16 i);
const char* to_string(U32 i);
const char* to_string(I32 i);
const char* to_string(U64 i);
const char* to_string(I64 i);
const char* to_string(void* i);

const char* to_string_hex(U16 i);
const char* to_string_hex(U32 i);
const char* to_string_hex(U64 i);
const char* to_string_hex_trim(U16 i);
const char* to_string_hex_trim(U32 i);
const char* to_string_hex_trim(U64 i);

//NOTE:HACK: int32_t and uint32_t aren't typedefs of int and unsigned int literals with this compiler, even though it's 32bit
#if defined(ARCH_RASPI1) or defined(ARCH_RASPI2)
	inline const char* to_string(int i) { return to_string((I32)i); }
	inline const char* to_string(unsigned int i) { return to_string((U32)i); }
	inline const char* to_string_hex(unsigned int i) { return to_string_hex((U32)i); }
	inline const char* to_string_hex_trim(unsigned int i) { return to_string_hex_trim((U32)i); }
#endif
