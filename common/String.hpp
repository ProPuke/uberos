#pragma once

#include "maths.hpp"
#include "stdlib.hpp"
#include "types.hpp"

template <typename Type>
struct String {
	static const inline auto localBufferSize = max<U32>(8, 16/sizeof(Type));

	U32 length = 0;
	union {
		struct {
			Type *data;
			U32 allocated; // not including null byte
		};
		Type localData[localBufferSize];
	};

	constexpr /**/ String() {
		localData[0] = '\0';
	}

	/**/ String(const char *data){
		localData[0] = '\0';

		if(data){
			append(data);
		}
	}

	// template<typename ...Params>
	// /**/ String(Params ...params){
	// 	U32 requiredLength = //TODO:total max length of all params

	// 	//TODO: append each with toString()
	// }

	/**/~String(){
		if(!is_local_string()){
			delete data;
		}
	}

	void resize(U32 newSize){
		const auto isLocal = is_local_string();

		if(newSize+1<=localBufferSize){
			if(isLocal) return; // nothing to do

			memcpy(localData, data, length);
			localData[length] = '\0';
			return;
		}

		if(isLocal){
			data = new Type[(allocated=newSize)+1];
			memcpy(data, localData, length);
			data[length] = '\0';

		}else{
			if(newSize==allocated) return;

			auto newData = new Type[(allocated=newSize)+1];
			memcpy(newData, data, min(length, newSize));
			newData[length] = '\0';

			delete data;
			data = newData;
		}
	}

	void append(Type c){
		if(is_local_string()){
			if(length+1+1>localBufferSize){
				resize(length+1);
			}

		}else{
			if(length+1>allocated){
				resize(length+1);
			}
		}

		auto data = get_data();
		data[length++] = c;
		data[length] = '\0';
	}

	void append(const char *source){
		auto sourceLength = strlen(source);
		if(sourceLength<1) return;

		if(is_local_string()){
			if(length+sourceLength+1>localBufferSize){
				resize(length+sourceLength);
			}

		}else{
			if(length+sourceLength+1>allocated){
				resize(length+sourceLength);
			}
		}

		memcpy(&get_data()[length], source, sourceLength+1);
		length += sourceLength;
	}

	void append(const String &source){
		if(source.length<1) return;

		if(is_local_string()){
			if(length+source.length+1>localBufferSize){
				resize(length+source.length);
			}

		}else{
			if(length+source.length+1>allocated){
				resize(length+source.length);
			}
		}

		memcpy(&get_data()[length], source.get_data(), source.length+1);
		length += source.length;
	}

	void insert(U32 index, Type c){
		if(is_local_string()){
			if(length+1+1>localBufferSize){
				resize(length+1);
			}

		}else{
			if(length+1>allocated){
				resize(length+1);
			}
		}

		auto data = get_data();

		//TODO:optimise the right side is moved twice, once by resize(), then here again. Ideally we would resize without the full memcpy
		memmove(&data[index+1], memmove(&data[index]), length++-index+1);
		data[index] = c;
	}

	auto operator[](size_t i) const -> Type  { return get_data()[i]; }
	auto operator[](size_t i) /* */ -> Type& { return get_data()[i]; }

	auto operator==(const String &b) const -> bool {
		if(length!=b.length) return false;
		return memcmp(get_data(), b.get_data(), length*sizeof(Type))==0;
	}

	auto operator==(const Type *b) const -> bool {
		auto bLength = strlen(b);
		if(length!=bLength) return false;
		return memcmp(get_data(), b, bLength*sizeof(Type))==0;
	}

	auto is_local_string() const -> bool {
		return length+1<=localBufferSize;
	}

	auto get_data() const -> const char * {
		return is_local_string()?localData:data;
	}

	auto get_data() /* */ -> /* */ char * {
		return is_local_string()?localData:data;
	}
};

typedef String<C8> String8;
typedef String<C16> String16;
typedef String<C32> String32;
