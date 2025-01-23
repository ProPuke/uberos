#pragma once

#include <common/EventEmitter.hpp>
#include <common/LList.hpp>
#include <common/Try.hpp>

struct Driver;
struct DriverType;

namespace drivers {
	struct Event {
		enum struct Type {
			driver_started,
			driver_stopped
		} type;

		union {
			struct {
				Driver *driver;
			} driver_started;

			struct {
				Driver *driver;
				const char *error;
			} driver_stopped;
		};
	};

	extern EventEmitter<Event> events;

	void install_driver(Driver&);

	auto start_driver(Driver&) -> Try<>;
	auto stop_driver(Driver&) -> Try<>;
	auto restart_driver(Driver&) -> Try<>;

	auto find_first(DriverType&) -> Driver*;
	auto find_next(Driver &after, DriverType&) -> Driver*;

	template <typename T>
	auto find_first(DriverType &type) -> T* { return (T*)find_first(type); }
	template <typename T>
	auto find_first() -> T* { return find_first<T>(T::typeInstance); }
	template <typename T>
	auto find_next(Driver &after, DriverType &type) -> T* { return (T*)find_next(after, type); }
	template <typename T>
	auto find_next(Driver &after) -> T* { return find_next<T>(after, T::typeInstance); }

	void print_driver_summary(const char *indent, Driver&);
	bool print_driver_details(const char *indent, Driver&, const char *beforeName="", const char *afterName="");

	auto is_memory_in_use(void*, size_t) -> bool;

	auto _on_interrupt(U8, const void *cpuState) -> const void*;
	void _on_irq(U8);

	void _subscribe_driver_to_irq(Driver&, U8 irq);
	void _unsubscribe_driver_from_irq(Driver&, U8 irq);

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

		auto begin() -> Iterator<Type> { return {find_first<Type>(type), type}; }
		auto end() -> Iterator<Type> { return {nullptr, type}; }
	};

	template <typename Type = Driver>
	inline auto iterate() -> Iterate<Type> { return Iterate<Type>(Type::typeInstance); }

	auto find_and_activate(DriverType&, Driver *onBehalf = nullptr) -> Driver*;

	template <typename Type>
	auto find_and_activate(Driver *onBehalf = nullptr) -> Type* {
		return (Type*)find_and_activate(Type::typeInstance, onBehalf);
	}
}
