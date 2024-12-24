#pragma once

#include "Array.hpp"

#include <common/maths.hpp>
#include <common/stdlib.hpp>

template <typename Type>
struct PodArray:Array<Type> {
	using Array<Type>::length;
	using Array<Type>::allocated;
	using Array<Type>::data;

	/**/ PodArray(U32 reserveSize=0):
		Array<Type>(reserveSize)
	{}

	/**/~PodArray(){}

	void resize(U32 newSize){
		newSize = max(max(length, (U32)1), newSize);
		if(newSize==allocated) return;

		const auto newData = (Type*)kmalloc(allocated=newSize);

		length = min(length, newSize);
		memcpy(newData, data, length*sizeof(Type));

		kfree(data);
		data = newData;
	}

	template <typename ...Params>
	void push_back(Params ...params){
		if(length+1>=allocated){
			resize(allocated+allocated/2);
		}
		new ((void*)&data[length++]) Type{params...};
	}

	// void push_front(Type &item){
	// 	if(length+1>=allocated){
	// 		//TODO:optimise:resize current involves a memmove, meaning there are 2 memmoves() rather than just 1
	// 		resize(allocated+allocated/2);
	// 	}
	// 	memmove(&data[1], &data[0], length++);
	// 	data[0] = &item;
	// }

	auto back() -> Type& {
		return data[length-1];
	}

	void remove_front() {
		if(length<1) return;

		memmove(&data[1], &data[0], --length);
	}

	void remove_back() {
		length--;
	}

	void remove(U32 index) {
		if(index>=length) return;

		if(index==0) return remove_front();
		if(index==length-1) return remove_back();

		memmove(&data[index], &data[index+1], --length-index);
	}

	// if you don't care about the value, remove*, don't pop* (thus we [[nodiscard]])

	[[nodiscard]] auto pop_front() -> Type {
		if(length<1) return nullptr;

		Type value = data[0];
		memmove(&data[1], &data[0], --length);

		return value;
	}

	[[nodiscard]] auto pop_back() -> Type {
		return data[--length];
	}

	[[nodiscard]] auto pop(U32 index) -> Type {
		if(index>=length) return nullptr;

		if(index==0) return remove_front();
		if(index==length-1) return remove_back();

		Type value = data[index];

		memmove(&data[index], &data[index+1], --length-index);

		return value;
	}

	// void insert(U32 index, Type *value){
	// 	if(length+1>=allocated){
	// 		//TODO:optimise:resize current involves a memmove, meaning there are 2 memmoves() rather than just 1
	// 		resize(allocated+allocated/2);
	// 	}

	// 	memmove(&data[index+1], memmove(&data[index]), length++-index);
	// }

	template <typename ...Params>
	void insert(U32 index, Params ...params) {
		if(length+1>=allocated){
			auto newData = (Type*)kmalloc(allocated=allocated+allocated/2+1);
			memmove(newData, data, index);
			memmove(&newData[index+1], &data[index], length-index);
			kfree(data);
			data = newData;

		}else{
			memmove(&data[index+1], &data[index], length-index);
		}

		new((void*)&data[index]) Type{params...};

		length++;
	}

	void shift_left(U32 count){
		if(count>=length) {
			clear();
			return;
		}

		auto newLength = length - count;
		memmove(&data[0], &data[count], newLength);
		length = newLength;
	}

	void clear(){
		length = 0;
	}
};
