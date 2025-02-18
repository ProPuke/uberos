#pragma once

#include "config.h"

#include <stddef.h>
#include <cstdint>
// #include <type_traits>

// typedef uint8_t U8;
// typedef int8_t I8;
// typedef uint16_t U16;
// typedef int16_t I16;
// typedef uint32_t U32;
// typedef int32_t I32;
// typedef uint64_t U64;
// typedef int64_t I64;

typedef unsigned char U8;
typedef signed char I8;
typedef unsigned short U16;
typedef signed short I16;
typedef unsigned int U32;
typedef signed int I32;
#if __WORDSIZE == 64
	typedef unsigned long int U64;
	typedef signed long int I64;
#else
	typedef unsigned long long int U64;
	typedef signed long long int I64;
#endif

#ifdef HAS_128BIT
	typedef unsigned __int128 U128;
	typedef __int128 I128;
#endif

typedef uintmax_t UMax;
typedef intmax_t IMax;

typedef uintptr_t UPtr;

typedef uint_fast8_t  U8Fast;
typedef uint_fast16_t U16Fast;
typedef uint_fast32_t U32Fast;
typedef uint_fast64_t U64Fast;

typedef int_fast8_t  I8Fast;
typedef int_fast16_t I16Fast;
typedef int_fast32_t I32Fast;
typedef int_fast64_t I64Fast;

typedef float  F32;
typedef double F64;

typedef char     C8;
typedef char16_t C16;
typedef char32_t C32;

#if defined(_32BIT)
	typedef U32 Reg;
#elif defined(_64BIT)
	typedef U64 Reg;
#elif defined(_128BIT)
	typedef U128 Reg;
#endif

#include "Box.hpp"
#include "Optional.hpp"
#include "Pointer.hpp"
