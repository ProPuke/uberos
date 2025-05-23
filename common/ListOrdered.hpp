#pragma once

#include "types.hpp"
#include "maths.hpp"

template <typename Type>
struct ListOrdered {
	U32 length = 0;
	U32 allocated;
	Type *data;

	constexpr /**/ ListOrdered():
		allocated(0),
		data(nullptr)
	{}

	/**/ ListOrdered(U32 reserveSize):
		allocated(reserveSize),
		data(reserveSize?new Type[reserveSize]:nullptr)
	{}

	/**/ ListOrdered(const ListOrdered &copy):
		length(copy.length),
		allocated(copy.length),
		data(nullptr)
	{
		if(allocated>0){
			data = new Type[allocated];
			memcpy(data, copy.data, allocated*sizeof(Type));
		}
	}

	/**/~ListOrdered(){
		delete data;
	}

	void resize(U32 newSize){
		newSize = maths::max(maths::max(length, (U32)1), newSize);
		if(newSize==allocated) return;

		auto newData = new Type[allocated=newSize];
		memcpy(newData, data, length*sizeof(Type));

		delete data;
		data = newData;
	}

	void push_back(const Type &item){
		if(length+1>=allocated){
			resize(length+1+length/2);
		}
		data[length++] = item;
	}

	void push_front(const Type &item){
		if(length+1>=allocated){
			//TODO:optimise:resize current involves a memmove, meaning there are 2 memmoves() rather than just 1
			resize(length+1+length/2);
		}
		memmove(&data[1], &data[0], (length++)*sizeof(Type));
		data[0] = item;
	}

	Type pop_back(){
		debug::assert(length>0);

		return data[--length];
	}

	Type pop_front(){
		debug::assert(length>0);

		Type value = data[0];
		memmove(&data[1], &data[0], (--length)*sizeof(Type));

		return value;
	}

	Type pop(U32 index){
		debug::assert(index<length);

		if(index==0) return pop_front();
		if(index==length-1) return pop_back();

		Type value = data[index];

		memmove(&data[index], &data[index+1], (--length-index)*sizeof(Type));

		return value;
	}

	void remove(U32 index){
		debug::assert(index<length);

		--length;

		if(index<length){
			memmove(&data[index], &data[index+1], (length-index)*sizeof(Type));
		}
	}

	void insert(U32 index, Type value){
		debug::assert(index<=length);

		if(length+1>=allocated){
			//TODO:optimise:resize current involves a memmove, meaning there are 2 memmoves() rather than just 1
			resize(length+1+length/2);
		}

		memmove(&data[index+1], &data[index], (length++-index)*sizeof(Type));
	}

	void clear(){
		length = 0;
	}

	auto begin() -> Type* { return &data[0]; }
	auto end() -> Type* { return &data[length]; }

	auto operator[](U32 index) -> Type& { return data[index]; }
	auto operator[](U32 index) const -> const Type& { return data[index]; }

	auto operator=(const ListOrdered &copy) -> ListOrdered& {
		if(&copy==this) return *this;

		delete data;

		length = copy.length;
		allocated = copy.length;
		data = nullptr;

		if(allocated>0){
			data = new Type[allocated];
			memcpy(data, copy.data, allocated*sizeof(Type));
		}

		return *this;
	}
};
