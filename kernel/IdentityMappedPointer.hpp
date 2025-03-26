#pragma once

#include <kernel/PhysicalPointer.hpp>

template <typename Address, typename Type>
struct IdentityMappedPointer: PhysicalPointer<Address, Type> {
	typedef PhysicalPointer<Address, Type> Super;

	// constexpr /*    */ /**/ IdentityMappedPointer() {}
	// constexpr explicit /**/ IdentityMappedPointer(const Type *value):
	// 	Super(value)
	// {}
	// constexpr explicit /**/ IdentityMappedPointer(Address address):
	// 	Super(address)
	// {}

	// constexpr /*    */ /**/ IdentityMappedPointer() {}
	// constexpr explicit /**/ IdentityMappedPointer(const Type *value):
	// 	address(value)
	// {}
	// constexpr explicit /**/ IdentityMappedPointer(Address address):
	// 	address(address)
	// {}

	auto operator+(UPtr offset) -> IdentityMappedPointer { return IdentityMappedPointer{this->address+offset}; }
	auto operator+=(UPtr offset) -> IdentityMappedPointer& {
		this->address += offset;
		return *this;
	}

	auto operator-(UPtr offset) -> IdentityMappedPointer { return IdentityMappedPointer{this->address-offset}; }
	auto operator-=(UPtr offset) -> IdentityMappedPointer& {
		this->address -= offset;
		return *this;
	}

	auto operator->() -> Type* { return (Type*)this->address; }

	operator Type*() { return (Type*)this->address; }
	explicit operator bool() { return !!this->address; }

	template <typename Address2, typename Type2>
	auto operator<=>(const IdentityMappedPointer<Address2, Type2> &other) const { return this->address <=> other.address; }
	template <typename Address2, typename Type2>
	auto operator==(const IdentityMappedPointer<Address2, Type2> &other) const { return this->address == other.address; }
	template <typename Address2, typename Type2>
	auto operator!=(const IdentityMappedPointer<Address2, Type2> &other) const { return this->address != other.address; }

	auto operator<=>(const Type *pointer) const { return this->address <=> pointer; }
	auto operator==(const Type *pointer) const { return this->address == pointer; }
	auto operator!=(const Type *pointer) const { return this->address != pointer; }

	auto operator=(const Type *value) -> IdentityMappedPointer& { this->address = (UPtr)value; return *this; }
} __attribute__((packed));

template <typename T>
using IdentityMapped = IdentityMappedPointer<UPtr, T>;

template <typename T>
using IdentityMapped8 = IdentityMappedPointer<U8, T>;

template <typename T>
using IdentityMapped16 = IdentityMappedPointer<U16, T>;

template <typename T>
using IdentityMapped32 = IdentityMappedPointer<U32, T>;

template <typename T>
using IdentityMapped64 = IdentityMappedPointer<U64, T>;
