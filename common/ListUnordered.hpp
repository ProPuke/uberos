#pragma once

#include "types.hpp"
#include "maths.hpp"

template <typename Type>
struct ListUnordered {
	U32 length = 0;
	U32 allocated;
	Type **data;

	/**/ ListUnordered(U32 reserveSize=32):
		allocated(reserveSize),
		data(new Type*[reserveSize])
	{}

	/**/~ListUnordered(){
		delete data;
	}

	void resize(U32 newSize){
		newSize = maths::max(maths::max(length, (U32)1), newSize);
		if(newSize==allocated) return;

		Type **newData = new Type*[allocated=newSize];
		memcpy(newData, data, length);

		delete data;
		data = newData;
	}

	void push(Type &item){
		if(length+1>=allocated){
			resize(allocated+allocated/2);
		}
		data[length++] = &item;
	}

	Type* pop(){
		if(length<1) return nullptr;

		return data[length-- -1];
	}

	Type* pop(U32 index){
		if(index>=length) return nullptr;

		if(length>1){
			data[index] = data[length-- -1];
		}else{
			length--;
		}
	}

	void clear(){
		length = 0;
	}

	auto operator[](U32 index) -> Type& { return data[index]; }
	auto operator[](U32 index) const -> const Type& { return data[index]; }
};
