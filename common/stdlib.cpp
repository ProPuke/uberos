#include "stdlib.hpp"

#include <common/maths.hpp>

#ifndef USE_STDLIB_ASM
	extern "C" void* __attribute__ ((optimize(1))) memcpy(void *__restrict dest, const void *__restrict src, size_t bytes) {
		if(dest==src) return dest;
		return memcpy_forwards(dest, src, bytes);
	}
#endif

extern "C" void* __attribute__ ((optimize(1))) memcpy_aligned(void *__restrict dest, const void *__restrict src, size_t bytes) {
	if(dest==src) return dest;
	return memcpy_forwards(dest, src, bytes);
}

extern "C" void* __attribute__ ((optimize(1))) memcpy_forwards(void *__restrict dest, const void *__restrict src, size_t bytes) {
	char *d = (char*)dest, *s = (char*)src;
	while(bytes--) *d++ = *s++;
	return dest;
}

extern "C" void* __attribute__ ((optimize(1))) memcpy_backwards(void *__restrict dest, const void *__restrict src, size_t bytes) {
	char *d = (char*)dest+bytes, *s = (char*)src+bytes;
	while(bytes--) *d-- = *s--;
	return dest;
}

#ifndef USE_STDLIB_ASM
	extern "C" void* __attribute__ ((optimize(1))) memmove(void *dest, const void *src, size_t bytes) {
		if(dest>=src){
			return memcpy_backwards(dest, src, bytes);
		}else{
			return memcpy_forwards(dest, src, bytes);
		}
	}
#endif

#ifndef USE_STDLIB_ASM
	extern "C" int __attribute__ ((optimize(1))) memcmp(const void *a, const void *b, size_t bytes) {
		while(bytes--){
			const auto diff = *((C8*)a)-*((C8*)b);
			a = (C8*)a+1;
			b = (C8*)b+1;
			if(diff){
				return diff;
			}
		}
		return 0;
	}
#endif

extern "C" void __attribute__ ((optimize(1))) bzero(void *dest, size_t bytes) {
	char *d = (char*)dest;
	while(bytes--) *d++ = 0;
}

#ifndef USE_STDLIB_ASM
	extern "C" size_t __attribute__ ((optimize(1))) strlen(const C8 *str) {
		U64 length = 0;
		while(*str){
			length++;
			str++;
		}
		return length;
	}
#endif

extern "C" char* __attribute__ ((optimize(1))) strcat(char *destination, const char *source) {
	strcpy(destination+strlen(destination), source);
	return destination;
}

#ifndef USE_STDLIB_ASM
	extern "C" char* __attribute__ ((optimize(1))) strcpy(char *__restrict destination, const char *__restrict source) {
		char *x = destination;
		while(*source){
			*x++ = *source++;
		}
		*x = '\0';
		return destination;
	}
#endif

#ifndef USE_STDLIB_ASM
	extern "C" int __attribute__ ((optimize(1))) strcmp(const char* str1, const char* str2) {
		while(*str1&&*str2){
			int diff = *str1-*str2;
			if(diff) return diff;
			str1++;
			str2++;
		}
		
		return *str1?+1:*str2?-1:0;
	}
#endif

constexpr unsigned digits_binary(unsigned bits, bool isSigned){
	return bits*8-(isSigned?1:0);
}

constexpr unsigned digits_decimal(unsigned bits, bool isSigned){
	return digits_binary(bits, isSigned)*643/2136+1;
}

constexpr unsigned digits_hex(unsigned bits, bool isSigned){
	return (digits_binary(bits, isSigned)+3)/4;
}

template <typename Type>
inline const char* _utoa(Type number){
	static char buffer[digits_decimal(sizeof(number), false)+1] = {0};
	buffer[sizeof(buffer)-1] = 0;

	char *end = &buffer[sizeof(buffer)-2];
	char *character = end;
	do {
		*character--=number%10+'0';
		number/=10;
	}while(number>0);

	return character+1;
}


template <typename Type>
inline const char* _utoahex(Type number){
	static char buffer[2+digits_hex(sizeof(number), false)+1] = {0};
	buffer[0] = '0';
	buffer[1] = 'x';
	buffer[sizeof(buffer)-1] = 0;

	const char digits[] = "0123456789ABCDEF";

	char *end = &buffer[sizeof(buffer)-2];
	char *character = end;
	do {
		*character--=digits[number%16];
		number/=16;
	}while(number>0);

	while(character>&buffer[1]){
		*character--='0';
	}

	return buffer;
}

template <typename Type>
inline const char* _utoahextrim(Type number){
	static char buffer[2+digits_hex(sizeof(number), false)+1] = {0};
	buffer[sizeof(buffer)-1] = 0;

	const char digits[] = "0123456789ABCDEF";

	char *end = &buffer[sizeof(buffer)-2];
	char *character = end;
	do {
		*character--=digits[number%16];
		number/=16;
	}while(number>0);

	*character--='x';
	*character--='0';

	return character+1;
}

template <typename Type>
inline char* _itoa(Type number, char *buffer = nullptr){
	static char internalBuffer[digits_decimal(sizeof(number), false)+1] = {0};
	if(!buffer) buffer = internalBuffer;
	
	buffer[sizeof(internalBuffer)-1] = 0;

	bool isNegative = number<0;
	unsigned numberAbs = isNegative?-number:number;

	char *end = &buffer[sizeof(internalBuffer)-2];
	char *character = end;
	do {
		*character--=numberAbs%10+'0';
		numberAbs/=10;
	}while(numberAbs>0);

	if(isNegative) *character--='-';

	return character+1;
}

template <typename Type>
inline const char* _ftoa(Type number) {
	Type abs = maths::abs(number);
	I64 i = (I64)number;
	U32 f = (abs-(I64)abs)*1000+0.5; // 3 decimal places

	if(f>999) return _itoa(i+maths::sign(number));
	if(f==0) return _itoa(i);

	static char internalBuffer[1+digits_decimal(sizeof(i), false)+1+digits_decimal(sizeof(f), false)] = {0};
	char *buffer = internalBuffer;

	buffer = _itoa(i, buffer);

	auto decimalDigits = _itoa(f);
	U32 decimalPos = strlen(buffer); //TODO: avoid this excess strlen walk?
	buffer[decimalPos] = '.';

	strcpy(&buffer[decimalPos+1], decimalDigits);

	return buffer;
}

auto utoa(U16 x) -> const char* { return _utoa(x); }
auto itoa(I16 x) -> const char* { return _itoa(x); }
auto utoa(U32 x) -> const char* { return _utoa(x); }
auto itoa(I32 x) -> const char* { return _itoa(x); }
auto utoa(U64 x) -> const char* { return _utoa(x); }
auto itoa(I64 x) -> const char* { return _itoa(x); }
auto ftoa(F32 x) -> const char* { return _ftoa(x); }
auto ftoa(F64 x) -> const char* { return _ftoa(x); }

auto to_string_hex(U8 x)  -> const char* { return _utoahex(x); }
auto to_string_hex(U16 x) -> const char*  { return _utoahex(x); }
auto to_string_hex(U32 x) -> const char*  { return _utoahex(x); }
auto to_string_hex(U64 x) -> const char*  { return _utoahex(x); }
auto to_string_hex_trim(U8 x)  -> const char* { return _utoahextrim(x); }
auto to_string_hex_trim(U16 x) -> const char*  { return _utoahextrim(x); }
auto to_string_hex_trim(U32 x) -> const char*  { return _utoahextrim(x); }
auto to_string_hex_trim(U64 x) -> const char*  { return _utoahextrim(x); }
