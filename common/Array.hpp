#pragma once

#ifdef KERNEL
	#include <kernel/memory.hpp>
#endif

#include <common/types.hpp>

template <typename Type>
struct Array {
	U32 length = 0;
	U32 allocated;
	Type *data;

	/**/ Array(U32 reserveSize=0):
		allocated(reserveSize)
	{
		data = reserveSize?new Type[reserveSize]:nullptr;
	}

	/**/~Array(){
		delete data;
	}

	auto begin() -> Type* { return &data[0]; }
	auto end() -> Type* { return &data[length]; }

	auto operator[](U32 index) -> Type& { return data[index]; }
	auto operator[](U32 index) const -> const Type& { return data[index]; }
};
