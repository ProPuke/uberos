#pragma once

#include <common/LList.hpp>

struct Driver;

template <typename Type = Driver>
struct DriverReference;

template <>
struct DriverReference<Driver>: LListItem<DriverReference<Driver>> {
	typedef void(*Callback)(void*);

	Driver *driver = nullptr;
	Callback onTerminated = nullptr;
	void *onTerminatedData = nullptr;
	/**/ DriverReference();
	/**/ DriverReference(Driver*, Callback onTerminated, void *onTerminatedData);
	/**/ DriverReference(const DriverReference&);
	/**/~DriverReference();

	auto operator=(const DriverReference&) -> DriverReference&;
	auto operator=(Driver*) -> DriverReference&;

	explicit operator bool() { return !!driver; }

	void set_on_terminated(Callback callback, void *data) { onTerminated = callback; onTerminatedData = data; }

	void terminate();
};

template <typename Type>
struct DriverReference: DriverReference<Driver> {
	typedef DriverReference<Driver> Super;

	/**/ DriverReference(): Super() {}
	/**/ DriverReference(const DriverReference &copy):Super(copy) {}
	/**/ DriverReference(Type *type, Callback onTerminated, void *onTerminatedData):
		Super(type, onTerminated, onTerminatedData)
	{}

	auto operator->() -> Type* { return (Type*)driver; }

	auto operator=(const DriverReference &copy) -> DriverReference& { Super::operator=(copy); return *this; }
	auto operator=(Driver *driver) -> DriverReference& { Super::operator=(driver); return *this; }

	explicit operator bool() { return Super::operator bool(); }
};

template <typename Type>
struct AutomaticDriverReference: DriverReference<Type> {
	auto get() -> Type*;

	auto operator->() -> Type* { return get(); }

	using DriverReference<Type>::operator=;

	explicit operator bool() { return !!get(); }

protected:
	bool isResolving = false; // prevent recursion
};

#include <kernel/drivers.hpp>

template <typename Type>
inline auto AutomaticDriverReference<Type>::get() -> Type* {
	if(!this->driver && !isResolving) {
		isResolving = true;
		(*this) = drivers::find_and_activate<Type>();
		isResolving = false;
	}

	return (Type*)this->driver;
}
