#pragma once

#include <common/LList.hpp>

struct Driver;
struct DriverType;

namespace drivers {
	void install_driver(Driver&, bool activate);

	void enable_driver(Driver&);
	auto activate_driver(Driver&) -> bool;
	auto stop_driver(Driver&) -> bool;
	auto restart_driver(Driver&) -> bool;

	auto find_first(DriverType&) -> Driver*;
	auto find_next(Driver &after, DriverType&) -> Driver*;

	template <typename T>
	auto find_first() -> T* { return (T*)find_first(T::driverType); }
	template <typename T>
	auto find_next(Driver &after, DriverType &type) -> T* { return (T*)find_next(after, type); }

	void print_driver_summary(const char *indent, Driver&);
	bool print_driver_details(const char *indent, Driver&, const char *beforeName="", const char *afterName="");

	auto is_memory_in_use(void*, size_t) -> bool;

	auto _on_interrupt(U8, const void *cpuState) -> const void*;
	void _on_irq(U8);

	template <typename Type>
	struct Iterator {
		Type *item;
		DriverType &type;

		/**/ Iterator(Type *item, DriverType &type):item(item),type(type){}

		auto operator*() -> Type& { return *item; }
		auto operator->() -> Type* { return item; }
		auto operator++() -> Iterator& { item = find_next<Type>(*item, type); return *this; }
		auto operator++(int x) -> Iterator& { while(item&&x>0) item = find_next(*item, type); return *this; }
		auto operator==(const Iterator &other) -> bool { return item==other.item; }
		auto operator!=(const Iterator &other) -> bool { return item!=other.item; }
	};

	template <typename Type>
	struct Iterate {
		DriverType &type;

		/**/ Iterate(DriverType &type):type(type){}

		auto begin() -> Iterator<Type> { return {find_first<Type>(), type}; }
		auto end() -> Iterator<Type> { return {nullptr, type}; }
	};

	template <typename Type>
	inline auto iterate() -> Iterate<Type> { return Iterate<Type>(Type::driverType); }

	template <typename Type>
	auto find_and_activate(Driver *onBehalf = nullptr) -> Type* {
		return (Type*)find_and_activate(Type::driverType, onBehalf);
	}

	auto find_and_activate(DriverType&, Driver *onBehalf = nullptr) -> Driver*;
}
