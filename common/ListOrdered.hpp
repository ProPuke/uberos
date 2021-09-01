#pragma once

#include "types.hpp"
#include "maths.hpp"

template <typename Type>
struct ListOrdered {
	U32 length;
	U32 allocated;
	Type *data;

	/**/ ListOrdered(U32 reserveSize=32):
		length(0),
		allocated(reserveSize),
		data(new Type*[reserveSize])
	{}

	/**/~ListOrdered(){
		delete data;
	}

	void resize(U32 newSize){
		newSize = max(max(length, (U32)1), newSize);
		if(newSize==allocated) return;

		auto newData = new Type**[allocated=newSize];
		memcpy(newData, data, length);

		delete data;
		data = newData;
	}

	void push_back(Type &item){
		if(length+1>=allocated){
			resize(allocated+allocated/2);
		}
		data[length++] = &item;
	}

	void push_front(Type &item){
		if(length+1>=allocated){
			//TODO:optimise:resize current involves a memmove, meaning there are 2 memmoves() rather than just 1
			resize(allocated+allocated/2);
		}
		memmove(&data[1], &data[0], length++);
		data[0] = &item;
	}

	Type* pop_back(){
		if(length<1) return nullptr;

		return data[--length];
	}

	Type* pop_front(){
		if(length<1) return nullptr;

		Type *value = data[0];
		memmove(&data[1], &data[0], --length);

		return value;
	}

	Type* pop(U32 index){
		if(index>=length) return nullptr;

		if(index==0) return pop_front();
		if(index==length-1) return pop_back();

		Type *value = data[index];

		memmove(&data[index], &data[index+1], --length-index);

		return value;
	}

	void insert(U32 index, Type *value){
		if(length+1>=allocated){
			//TODO:optimise:resize current involves a memmove, meaning there are 2 memmoves() rather than just 1
			resize(allocated+allocated/2);
		}

		memmove(&data[index+1], memmove(&data[index]), length++-index);
	}
};
