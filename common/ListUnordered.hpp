#pragma once

#include "types.hpp"
#include "maths.hpp"

template <typename Type>
struct ListUnordered {
	U32 length = 0;
	U32 allocated;
	Type *data;

	constexpr /**/ ListUnordered():
		allocated(0),
		data(nullptr)
	{}

	/**/ ListUnordered(U32 reserveSize):
		allocated(reserveSize),
		data(reserveSize>0?new Type[reserveSize]:nullptr)
	{}

	/**/~ListUnordered(){
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

	auto push(const Type &item) -> Type& {
		if(length+1>=allocated){
			resize(length+1+length/2);
		}
		return data[length++] = item;
	}

	auto pop() -> Type {
		debug::assert(length>0);

		return data[length-- -1];
	}

	auto pop(U32 index) -> Type {
		debug::assert(index<length);

		length--;

		if(length>0){
			data[index] = data[length];
		}
	}

	void remove(U32 index){
		debug::assert(index<length);

		--length;

		if(index<length){
			data[index] = data[length];
		}
	}

	void clear(){
		length = 0;
	}

	auto begin() -> Type* { return &data[0]; }
	auto end() -> Type* { return &data[length]; }

	auto operator[](U32 index) -> Type& { return data[index]; }
	auto operator[](U32 index) const -> const Type& { return data[index]; }
};
