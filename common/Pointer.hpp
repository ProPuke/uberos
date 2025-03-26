#pragma once

#include <common/types.hpp>


template <typename Address, typename Type>
struct Pointer {
	constexpr /**/ Pointer() {}
	constexpr /**/ Pointer(Type *value):
		data((Address)(UPtr)value)
	{}

	Address data = 0;

	auto get() -> Type* { return (Type*)(UPtr)data; }
	auto get() const -> const Type* { return (const Type*)(UPtr)data; }
	void set(Type *value) { data = (Address)(UPtr)value; }

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
