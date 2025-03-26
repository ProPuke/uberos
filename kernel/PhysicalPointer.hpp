#pragma once

#include <common/types.hpp>

template <typename Address, typename Type>
struct PhysicalPointer {
	// constexpr /*    */ /**/ PhysicalPointer() {}
	// constexpr explicit /**/ PhysicalPointer(const Type *value):
	// 	address((UPtr)value)
	// {}
	// constexpr explicit /**/ PhysicalPointer(Address address):
	// 	address(address)
	// {}

	auto operator+(UPtr offset) -> PhysicalPointer { return PhysicalPointer{address+offset}; }
	auto operator+=(UPtr offset) -> PhysicalPointer& {
		address += offset;
		return *this;
	}

	auto operator-(UPtr offset) -> PhysicalPointer { return PhysicalPointer{address-offset}; }
	auto operator-=(UPtr offset) -> PhysicalPointer& {
		address -= offset;
		return *this;
	}

	template <typename Type2>
	auto as_type() -> PhysicalPointer<Address, Type2> { return PhysicalPointer<Address, Type2>{address}; }
	template <typename Address2>
	auto as_size() -> PhysicalPointer<Address2, Type> { return PhysicalPointer<Address2, Type>{(Address2)address}; }
	auto as_native() -> PhysicalPointer<UPtr, Type> { return as_size<UPtr>(); }
	template <typename Type2>
	auto as_native_type() -> PhysicalPointer<UPtr, Type2> { return as_type<Type2>().as_native(); }

	// automatically cast to void*
	operator PhysicalPointer<Address, void>() { return as_type<void>(); }

	explicit operator bool() { return !!address; }

	template <typename Address2, typename Type2>
	auto operator<=>(const PhysicalPointer<Address2, Type2> &other) const { return address <=> other.address; }
	template <typename Address2, typename Type2>
	auto operator==(const PhysicalPointer<Address2, Type2> &other) const { return address == other.address; }
	template <typename Address2, typename Type2>
	auto operator!=(const PhysicalPointer<Address2, Type2> &other) const { return address != other.address; }

	auto operator=(const Type *value) -> PhysicalPointer& { address = (UPtr)value; return *this; }

	Address address;
} __attribute__((packed));

template <typename T>
using Physical = PhysicalPointer<UPtr, T>;

template <typename T>
using Physical8 = PhysicalPointer<U8, T>;

template <typename T>
using Physical16 = PhysicalPointer<U16, T>;

template <typename T>
using Physical32 = PhysicalPointer<U32, T>;

template <typename T>
using Physical64 = PhysicalPointer<U64, T>;
