#include "stdlib.hpp"

#include <common/maths.hpp>

#ifndef HAS_UNALIGNED_ACCESS
	extern "C" void* memcpy(void *__restrict dest, const void *__restrict src, size_t bytes) {
		return memcpy_aligned(dest, src, bytes);
	}
#endif

extern "C" void* memcpy_aligned(void *__restrict dest, const void *__restrict src, size_t bytes) {
	if(dest==src) return dest;
	return memcpy_forwards_aligned(dest, src, bytes);
}

#ifdef HAS_128BIT
	extern "C" void* memcpy_forwards_aligned(void *__restrict dest, const void *__restrict src, size_t bytes) {
		auto d = (char*)dest;
		auto s = (const char*)src;

		if(bytes>=16&&(uintptr_t)s&15==(uintptr_t)d&15){
			while((uintptr_t)s&15){
				*d++ = *s++;
				bytes--;
			}
			while(bytes>=16){
				*(U128*)d = *(U128*)s;
				s += 16;
				d += 16;
				bytes -= 16;
			}
		}

		while(bytes--) *d++ = *s++;

		return dest;
	}
#else
	extern "C" void* memcpy_forwards_aligned(void *__restrict dest, const void *__restrict src, size_t bytes) {
		auto d = (char*)dest;
		auto s = (const char*)src;

		if(bytes>=8&&(uintptr_t)s&7==(uintptr_t)d&7){
			while((uintptr_t)s&7){
				*d++ = *s++;
				bytes--;
			}
			while(bytes>=8){
				*(U64*)d = *(U64*)s;
				s += 8;
				d += 8;
				bytes -= 8;
			}
		}

		while(bytes--) *d++ = *s++;

		return dest;
	}
#endif

extern "C" void* memcpy_backwards_aligned(void *__restrict dest, const void *__restrict src, size_t bytes) {
	auto d = (char*)dest+bytes;
	auto s = (const char*)src+bytes;

	while(bytes--) *d-- = *s--;

	return dest;
}

#ifndef HAS_UNALIGNED_ACCESS
	extern "C" void* memmove(void *dest, const void *src, size_t bytes) {
		if(dest>=src){
			return memcpy_backwards_aligned(dest, src, bytes);
		}else{
			return memcpy_forwards_aligned(dest, src, bytes);
		}
	}

	extern "C" int memcmp(const void *a, const void *b, size_t bytes) {
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

#ifdef HAS_UNALIGNED_ACCESS
	extern "C" void bzero(void *dest, size_t bytes) {
		memset(dest, 0, bytes);
	}
#else
	extern "C" void bzero(void *dest, size_t bytes) {
		char *d = (char*)dest;
		while(bytes--) *d++ = 0;
	}
#endif

#ifndef HAS_UNALIGNED_ACCESS
	extern "C" size_t strlen(const C8 *str) {
		U64 length = 0;
		while(*str){
			length++;
			str++;
		}
		return length;
	}
#endif

extern "C" char* strcat(char *destination, const char *source) {
	strcpy(destination+strlen(destination), source);
	return destination;
}

#ifndef HAS_UNALIGNED_ACCESS
	extern "C" char* strcpy(char *__restrict destination, const char *__restrict source) {
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
