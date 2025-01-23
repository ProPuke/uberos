#pragma once

#include "maths.hpp"
#include "stdlib.hpp"
#include "types.hpp"

template <typename Type>
struct String {
	static const size_t localBufferSize = max<size_t>(8, 16/sizeof(Type));

	U32 length = 0;
	Type *data = localData;
	union {
		U32 allocated;
		Type localData[localBufferSize];
	};

	/**/ String() {
		data[0] = '\0';
	}

	template<typename ...Params>
	/**/ String(Params ...params){
		U32 requiredLength = //TODO:total max length of all params

		//TODO: append each with toString()
	}

	/**/~String(){
		if(data!=localData){
			delete data;
		}
	}

	void resize(U32 newSize){
		newSize = max(max(length, localBufferSize), newSize);
		if(newSize==allocated) return;

		if(newSize==localBufferSize){
			memcpy(localData, data, length);
			return;
		}

		auto newData = new Type[allocated=newSize];
		memcpy(newData, data, length);

		if(allocated!=localBufferSize){
			delete data;
		}

		data = newData;
		allocated = newSize;
	}

	void append(Type c){
		if(data==localData){
			if(length+1>localBufferSize-1){
				resize(localBufferSize+localBufferSize/2);
			}
		}
		if(length+1>=allocated){
			resize(allocated+allocated/2);
		}
		data[length++] = c;
	}

	void append(const char *source){
		auto sourceLength = strlen(source);

		if(length+sourceLength>=allocated){
			resize(allocated+sourceLength);
		}
		memcpy(&data[length], source, sourceLength);
		length += sourceLength;
	}

	void append(const String &source){
		if(length+source.length>=allocated){
			resize(allocated+source.length);
		}
		memcpy(&data[length], source.data, source.length);
		length += source.length;
	}

	void insert(U32 index, Type c){
		if(length+1>=allocated){
			//TODO:optimise:resize current involves a memmove, meaning there are 2 memmoves() rather than just 1
			resize(allocated+allocated/2);
		}

		memmove(&data[index+1], memmove(&data[index]), length++-index);
	}

	auto operator[](size_t i) const -> Type { return data[i]; }
	auto operator[](size_t i) -> Type& { return data[i]; }

	operator==(const String &b) const {
		if(length!=b.length) return false;
		return memcmp(a.data, b.data, a.length*sizeof(Type))==0;
	}
};

typedef String<C8> String8;
typedef String<C16> String16;
typedef String<C32> String32;
