#pragma once

#include "config.h"

#include <stdint.h>
#include <stddef.h>

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef unsigned __int128 U128;

typedef int8_t  I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;
typedef __int128 I128;

typedef uint_fast8_t  FastU8;
typedef uint_fast16_t FastU16;
typedef uint_fast32_t FastU32;
typedef uint_fast64_t FastU64;

typedef int_fast8_t  FastI8;
typedef int_fast16_t FastI16;
typedef int_fast32_t FastI32;
typedef int_fast64_t FastI64;

typedef float  F32;
typedef double F64;

typedef char     C8;
typedef char16_t C16;
typedef char32_t C32;

#if defined(ARCH_ARM32)
	#define _32BIT
	typedef U32 Reg;
#elif defined(ARCH_ARM64) or defined(__x86_64__)
	#define _64BIT
	typedef U64 Reg;
#else
	#error Unsupported architecture
#endif
