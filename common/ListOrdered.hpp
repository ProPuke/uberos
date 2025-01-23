#pragma once

#include "types.hpp"
#include "maths.hpp"

template <typename Type>
struct ListOrdered {
	U32 length = 0;
	U32 allocated;
	Type *data;

	/**/ ListOrdered(U32 reserveSize=32):
		allocated(reserveSize),
		data(reserveSize?new Type[reserveSize]:nullptr)
	{}

	/**/~ListOrdered(){
		delete data;
	}

	void resize(U32 newSize){
		newSize = maths::max(maths::max(length, (U32)1), newSize);
		if(newSize==allocated) return;

		auto newData = new Type[allocated=newSize];
		memcpy(newData, data, length);

		delete data;
		data = newData;
	}

	void push_back(const Type &item){
		if(length+1>=allocated){
			resize(allocated+allocated/2);
		}
		data[length++] = item;
	}

	void push_front(const Type &item){
		if(length+1>=allocated){
			//TODO:optimise:resize current involves a memmove, meaning there are 2 memmoves() rather than just 1
			resize(allocated+allocated/2);
		}
		memmove(&data[1], &data[0], length++);
		data[0] = item;
	}

	Type pop_back(){
		debug::assert(length>0);

		return data[--length];
	}

	Type pop_front(){
		debug::assert(length>0);

		Type value = data[0];
		memmove(&data[1], &data[0], --length);

		return value;
	}

	Type pop(U32 index){
		debug::assert(index<length);

		if(index==0) return pop_front();
		if(index==length-1) return pop_back();

		Type value = data[index];

		memmove(&data[index], &data[index+1], --length-index);

		return value;
	}

	void remove(U32 index){
		debug::assert(index<length);

		--length;

		if(index<length){
			memmove(&data[index], &data[index+1], length-index);
		}
	}

	void insert(U32 index, Type value){
		debug::assert(index<=length);

		if(length+1>=allocated){
			//TODO:optimise:resize current involves a memmove, meaning there are 2 memmoves() rather than just 1
			resize(allocated+allocated/2);
		}

		memmove(&data[index+1], memmove(&data[index]), length++-index);
	}

	void clear(){
		length = 0;
	}

	auto begin() -> Type* { return &data[0]; }
	auto end() -> Type* { return &data[length]; }

	auto operator[](U32 index) -> Type& { return data[index]; }
	auto operator[](U32 index) const -> const Type& { return data[index]; }
};
