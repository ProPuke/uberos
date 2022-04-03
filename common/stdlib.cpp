#include "stdlib.hpp"

#include <common/maths.hpp>

#include <cstddef>

extern "C" void memcpy(void *dest, const void *src, unsigned bytes) {
	if(dest==src) return;
	memcpy_forwards(dest, src, bytes);
}

extern "C" void memcpy_forwards(void *dest, const void *src, unsigned bytes) {
	char *d = (char*)dest, *s = (char*)src;
	while(bytes--) *d++ = *s++;
}

extern "C" void memcpy_backwards(void *dest, const void *src, unsigned bytes) {
	char *d = (char*)dest+bytes, *s = (char*)src+bytes;
	while(bytes--) *d-- = *s--;
}

extern "C" void memmove(void *dest, const void *src, unsigned bytes) {
	if(dest>=src){
		memcpy_backwards(dest, src, bytes);
	}else{
		memcpy_forwards(dest, src, bytes);
	}
}

extern "C" int memcmp(const void *a, const void *b, unsigned bytes) {
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

extern "C" void bzero(void *dest, unsigned bytes) {
	char *d = (char*)dest;
	while(bytes--) *d++ = 0;
}

extern "C" unsigned strlen(const C8 *str) {
	U64 length = 0;
	while(*str){
		length++;
		str++;
	}
	return length;
}

extern "C" char* strcat(char *destination, const char *source) {
	return strcpy(destination+strlen(destination), source);
}

extern "C" char* strcpy(char *destination, const char *source) {
	char *x = destination;
	while(*source){
		*x++ = *source++;
	}
	*x = '\0';
	return destination;
}

extern "C" int strcmp(const char* str1, const char* str2) {
	while(*str1&&*str2){
		int diff = *str1-*str2;
		if(diff) return diff;
		str1++;
		str2++;
	}
	
	return *str1?+1:*str2?-1:0;
}

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

auto to_string(U16 x) -> const char* { return _utoa(x); }
auto to_string(I16 x) -> const char* { return _itoa(x); }
auto to_string(U32 x) -> const char* { return _utoa(x); }
auto to_string(I32 x) -> const char* { return _itoa(x); }
auto to_string(U64 x) -> const char* { return _utoa(x); }
auto to_string(I64 x) -> const char* { return _itoa(x); }

auto to_string(F32 x) -> const char* { return _ftoa(x); }
auto to_string(F64 x) -> const char* { return _ftoa(x); }

auto to_string_hex(U8 x)  -> const char* { return _utoahex(x); }
auto to_string_hex(U16 x) -> const char*  { return _utoahex(x); }
auto to_string_hex(U32 x) -> const char*  { return _utoahex(x); }
auto to_string_hex(U64 x) -> const char*  { return _utoahex(x); }
auto to_string_hex_trim(U8 x)  -> const char* { return _utoahextrim(x); }
auto to_string_hex_trim(U16 x) -> const char*  { return _utoahextrim(x); }
auto to_string_hex_trim(U32 x) -> const char*  { return _utoahextrim(x); }
auto to_string_hex_trim(U64 x) -> const char*  { return _utoahextrim(x); }
