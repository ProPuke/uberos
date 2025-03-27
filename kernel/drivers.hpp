#pragma once

#include <common/DriverTypeId.hpp>

#include <common/EventEmitter.hpp>
#include <common/LList.hpp>
#include <common/Try.hpp>

struct Driver;
struct DriverType;

namespace drivers {
	struct Event {
		enum struct Type {
			driverStarted,
			driverStopped
		} type;

		union {
			struct {
				Driver *driver;
			} driverStarted;

			struct {
				Driver *driver;
				const char *error;
			} driverStopped;
		};
	};

	extern EventEmitter<Event> events;

	void install_driver(Driver&);

	auto start_driver(Driver&) -> Try<>;
	auto stop_driver(Driver&) -> Try<>;
	auto restart_driver(Driver&) -> Try<>;

	auto find_first(DriverTypeId) -> Driver*;
	auto find_next(Driver &after, DriverTypeId) -> Driver*;

	template <typename T>
	auto find_first(DriverTypeId typeId) -> T* { return (T*)find_first(typeId); }
	template <typename T>
	auto find_first() -> T* { return find_first<T>(T::typeInstance.id); }
	template <typename T>
	auto find_next(Driver &after, DriverTypeId typeId) -> T* { return (T*)find_next(after, typeId); }
	template <typename T>
	auto find_next(Driver &after) -> T* { return find_next<T>(after, T::typeInstance.id); }

	void print_driver_summary(const char *indent, Driver&);
	bool print_driver_details(const char *indent, Driver&, const char *beforeName="", const char *afterName="");

	auto is_memory_in_use(Physical<void>, size_t) -> bool;

	auto _on_interrupt(U8, const void *cpuState) -> const void*;
	void _on_irq(U8);

	auto _subscribe_driver_to_irq(Driver&, U8 irq) -> Try<>;
	void _unsubscribe_driver_from_irq(Driver&, U8 irq);

	template <typename Type>
	struct Iterator {
		Type *item;
		DriverTypeId typeId;

		/**/ Iterator(Type *item, DriverTypeId typeId):item(item),typeId(typeId){}

		auto operator*() -> Type& { return *item; }
		auto operator->() -> Type* { return item; }
		auto operator++() -> Iterator& { item = find_next<Type>(*item, typeId); return *this; }
		auto operator++(int x) -> Iterator& { while(item&&x>0) item = find_next(*item, typeId); return *this; }
		auto operator==(const Iterator &other) -> bool { return item==other.item; }
		auto operator!=(const Iterator &other) -> bool { return item!=other.item; }
	};

	template <typename Type>
	struct Iterate {
		DriverTypeId typeId;

		/**/ Iterate(DriverTypeId typeId):typeId(typeId){}

		auto begin() -> Iterator<Type> { return {find_first<Type>(typeId), typeId}; }
		auto end() -> Iterator<Type> { return {nullptr, typeId}; }
	};

	template <typename Type = Driver>
	inline auto iterate() -> Iterate<Type> { return Iterate<Type>(Type::typeInstance.id); }

	auto find_and_activate(DriverTypeId, Driver *onBehalf = nullptr) -> Driver*;
	auto find_active(DriverTypeId, Driver *onBehalf = nullptr) -> Driver*;

	template <typename Type>
	auto find_and_activate(Driver *onBehalf = nullptr) -> Type* {
		return (Type*)find_and_activate(Type::typeInstance.id, onBehalf);
	}

	template <typename Type>
	auto find_active(Driver *onBehalf = nullptr) -> Type* {
		return (Type*)find_active(Type::typeInstance.id, onBehalf);
	}
}
