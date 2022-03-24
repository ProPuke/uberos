#pragma once

#include <common/LList.hpp>

struct Driver;

namespace device {
	void install_device(Driver &device, bool enable);

	auto start_device(Driver &device) -> bool;
	auto stop_device(Driver &device) -> bool;
	auto restart_device(Driver &device) -> bool;

	auto find_first() -> Driver*;
	auto find_first_type(const char *type) -> Driver*;
	auto find_next_type(Driver &after, const char *type) -> Driver*;

	template <typename Type>
	struct Iterator {
		Type *device = nullptr;
		const char *type = nullptr;
		/**/ Iterator(Type *device, const char *type):device(device),type(type) {}
		/**/ Iterator(){}

		auto operator!=(const Iterator<Type>&op) { return device!=op.device; }
		auto operator*() const -> Type& { return *device; }
		auto operator++() -> Iterator<Type> { auto temp=*this; device=type?(Type*)find_next_type(*device, type):(Type*)device->next; return temp; }
	};

	template <typename Type>
	struct Iterate {
		const char *type = nullptr;
		/**/ Iterate(const char *type):type(type){}
		/**/ Iterate(){}
		auto begin() -> Iterator<Type>;
		auto end() -> Iterator<Type>;
	};

	template <typename Type> auto iterate_type(const char *type) { return Iterate<Type>(type); }
	template <typename Type> auto iterate_all() { return Iterate<Type>(); }

	template <typename Type> inline auto Iterate<Type>::begin() -> Iterator<Type> { return Iterator<Type>(type?(Type*)find_first_type(type):(Type*)find_first(), type); }
	template <typename Type> inline auto Iterate<Type>::end() -> Iterator<Type> { return Iterator<Type>(nullptr, type); }
}
