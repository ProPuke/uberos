#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t  I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef float  F32;
typedef double F64;

typedef char     C8;
typedef char16_t C16;
typedef char32_t C32;

#if defined(ARCH_ARM32)
	#define _32BIT
	typedef U32 Reg;
#elif defined(ARCH_ARM64)
	#define _64BIT
	typedef U64 Reg;
#else
	#error Unsupported architecture
#endif
