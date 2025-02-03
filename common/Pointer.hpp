#pragma once

#include "types.hpp"

template <typename Data, typename Type>
struct Pointer {
	/**/ Pointer() {}
	/**/ Pointer(Type *value):
		data((Data)(size_t)value)
	{}

	Data data{};

	auto get() -> Type* { return (Type*)(size_t)data; }
	auto get() const -> const Type* { return (const Type*)(size_t)data; }
	void set(Type *value) { data = (Data)(size_t)value; }

	auto operator->() { return get(); }
	auto operator->() const { return get(); }

	explicit operator bool() const { return get(); }

	operator Type*() { return get(); }
	operator const Type*() const { return get(); }

	auto operator=(Type *value) -> Pointer { set(value); return *this; }
} __attribute__((packed));

template <typename T>
using P8 = Pointer<U8, T>;

template <typename T>
using P16 = Pointer<U16, T>;

template <typename T>
using P32 = Pointer<U32, T>;
