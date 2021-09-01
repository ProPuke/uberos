#include "stdlib.hpp"

#include <cstddef>

void memcpy(void *dest, void *src, int bytes) {
	char *d = (char*)dest, *s = (char*)src;
	while(bytes--) *d++ = *s++;
}

void bzero(void *dest, int bytes) {
	char *d = (char*)dest;
	while(bytes--) *d++ = 0;
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
inline const char* _itoa(Type number){
	static char buffer[digits_decimal(sizeof(number), false)+1] = {0};
	buffer[sizeof(buffer)-1] = 0;

	bool isNegative = number<0;
	unsigned numberAbs = isNegative?-number:number;

	char *end = &buffer[sizeof(buffer)-2];
	char *character = end;
	do {
		*character--=numberAbs%10+'0';
		numberAbs/=10;
	}while(numberAbs>0);

	if(isNegative) *character--='-';

	return character+1;
}

const char* to_string(U16 x){ return _utoa(x); }
const char* to_string(I16 x){ return _itoa(x); }
const char* to_string(U32 x){ return _utoa(x); }
const char* to_string(I32 x){ return _itoa(x); }
const char* to_string(U64 x){ return _utoa(x); }
const char* to_string(I64 x){ return _itoa(x); }
const char* to_string(void* x){ return _utoahex((size_t)x); }

const char* to_string_hex(U16 x) { return _utoahex(x); }
const char* to_string_hex(U32 x) { return _utoahex(x); }
const char* to_string_hex(U64 x) { return _utoahex(x); }
const char* to_string_hex_trim(U16 x) { return _utoahextrim(x); }
const char* to_string_hex_trim(U32 x) { return _utoahextrim(x); }
const char* to_string_hex_trim(U64 x) { return _utoahextrim(x); }
