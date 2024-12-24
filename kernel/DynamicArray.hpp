#pragma once

#include "Array.hpp"

#include <common/maths.hpp>
#include <common/stdlib.hpp>

template <typename Type>
struct DynamicArray:Array<Type> {
	using Array<Type>::length;
	using Array<Type>::allocated;
	using Array<Type>::data;

	/**/ DynamicArray(U32 reserveSize=32):
		Array<Type>(reserveSize)
	{}

	/**/~DynamicArray(){
		for(auto i=0u;i<length;i++){
			data[i].~Type();
		}
	}

	void resize(U32 newSize){
		newSize = max(max(length, (U32)1), newSize);
		if(newSize==allocated) return;

		auto newData = (Type*)kmalloc(allocated=newSize);
		for(auto i=0u;i<length;i++){
			new ((void*)&newData[i]) Type(data[i]);
			data[i].~Type();
		}

		kfree(data);
		data = newData;
	}

	template <typename ...Params>
	void push_back(Params ...params){
		if(length+1>=allocated){
			resize(allocated+allocated/2);
		}
		new ((void*)&data[length++]) Type(params...);
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

	void pop_back(){
		data[--length].~Type();
	}

	// Type* pop_front(){
	// 	if(length<1) return nullptr;

	// 	Type *value = data[0];
	// 	memmove(&data[1], &data[0], --length);

	// 	return value;
	// }

	// Type* pop(U32 index){
	// 	if(index>=length) return nullptr;

	// 	if(index==0) return pop_front();
	// 	if(index==length-1) return pop_back();

	// 	Type *value = data[index];

	// 	memmove(&data[index], &data[index+1], --length-index);

	// 	return value;
	// }

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
			for(auto i=0u;i<index;i++){
				new ((void*)&newData[i]) Type(data[i]);
				data[i].~Type();
			}

			for(auto i=index;i<length;i++){
				new ((void*)&newData[i+1]) Type(data[i]);
				data[i].~Type();
			}

			kfree(data);
			data = newData;

		}else{
			if(index<length){
				for(auto i=length-1;i>=index;i--){
					new(&data[i+1]) Type(data[i]);
					data[i].~Type();
				}
			}
		}

		new((void*)&data[index]) Type(...params);

		length++;
	}

	void clear(){
		for(auto i=0u;i<length;i++){
			data[i].~Type();
		}
		length = 0;
	}
};
