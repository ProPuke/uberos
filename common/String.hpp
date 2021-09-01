#pragma once

#include "types.hpp"
#include "maths.hpp"

template <typename Type>
struct String {
	static const unsigned localBufferSize = max(8, 16/sizeof(Type));

	U32 length = 0;
	U32 allocated = localBufferSize;
	enum {
		Type *data;
		Type localData[localBufferSize];
	};

	/**/ String() {}

	template<typename ...Params>
	/**/ String(Params ...params){
		U32 requiredLength = //TODO:total max length of all params

		//TODO: append each with toString()
	}

	/**/~String(){
		if(allocated!=localBufferSize){
			delete data;
		}
	}

	void resize(U32 newSize){
		newSize = max(max(length, localBufferSize), newSize);
		if(newSize==allocated) return;

		if(newSize==localBufferSize){
			memcopy(localData, data, length);
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
};
