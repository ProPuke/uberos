#pragma once

#include <kernel/DriverApi.hpp>
#include <kernel/Log.hpp>
#include <kernel/memory.hpp>

#include <common/Bitmask.hpp>
#include <common/LList.hpp>
#include <common/Try.hpp>

#include <algorithm>

typedef U32 DriverTypeId;

struct DriverType {
	DriverTypeId id; // unique. must be cycled whenever this driver interface changes
	const char *name;
	const char *description;
	DriverType *parentType = nullptr;
};

#define DRIVER_TYPE(TYPE, ID, NAME, DESCRIPTION, PARENT) /**/\
	DRIVER_DECLARE_TYPE(ID, NAME, DESCRIPTION)\
	typedef PARENT Super;\
	static inline Log log{NAME};\
	protected:\
	/**/ TYPE(DriverApi::Startup startup):Super(startup) { DRIVER_DECLARE_INIT(); }\
	public:

#define DRIVER_TYPE_CUSTOM_CTOR(TYPE, ID, NAME, DESCRIPTION, PARENT) /**/\
	DRIVER_DECLARE_TYPE(ID, NAME, DESCRIPTION)\
	typedef PARENT Super;

#define DRIVER_INSTANCE(TYPE, ID, NAME, DESCRIPTION, PARENT) /**/\
	DRIVER_TYPE(TYPE, ID, NAME, DESCRIPTION, PARENT)\
	static TYPE instance;

#define DRIVER_DECLARE_TYPE(ID, NAME, DESCRIPTION) static inline DriverType typeInstance{ID, NAME, DESCRIPTION};
#define DRIVER_DECLARE_INIT() do { typeInstance.parentType = type; type = &typeInstance; } while(false)

template <typename Type = Driver>
struct DriverReference;

namespace drivers {
	auto _on_interrupt(U8 vector, const void *cpuState) -> const void*;
}

struct Driver: LListItem<Driver> {
	DRIVER_DECLARE_TYPE(0x1f24f156, "module", "Kernel Module")

	/*   */ /**/ Driver(DriverApi::Startup);
	virtual /**/~Driver() {}

	DriverType *type = nullptr;
	DriverApi api;
	LList<DriverReference<Driver>> references;

	auto is_type(DriverType &type) -> bool { return is_type(type.id); }
	auto is_type(DriverTypeId typeId) -> bool {
		for(auto type=this->type;type;type=type->parentType){
			if(type->id==typeId) return true;
		}
		return false;
	}

	template <typename Type>
	auto is_type() -> bool { return is_type(Type::typeInstance); }

	template <typename Type>
	auto as_type() -> Type* { return is_type<Type>()?(Type*)this:nullptr; }

	virtual auto can_stop_driver() -> bool { return true; }
	virtual auto can_restart_driver() -> bool { return true; }

	// virtual auto get_priority() -> U32 = 0;

	virtual void _on_irq(U8) {}

protected:

	friend class DriverApi;
	friend auto drivers::_on_interrupt(U8 vector, const void *cpuState) -> const void*;

	virtual auto _on_start() -> Try<> = 0;
	virtual auto _on_stop() -> Try<> = 0;

	virtual auto _on_interrupt(U8, const void *cpuState) -> const void* { return nullptr; }
};

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

	auto operator->() -> Type* { return (Type*)driver; }

	/**/ DriverReference(): Super() {}
	/**/ DriverReference(const DriverReference &copy):Super(copy) {}
	/**/ DriverReference(Type *type, Callback onTerminated, void *onTerminatedData):
		Super(type, onTerminated, onTerminatedData)
	{}

	auto operator=(const DriverReference &copy) -> DriverReference& { Super::operator=(copy); return *this; }
	auto operator=(Driver *driver) -> DriverReference& { Super::operator=(driver); return *this; }

	explicit operator bool() { return Super::operator bool(); }
};

// template<size_t length>
// struct StringLiteral {
// 	constexpr StringLiteral(const char (&str)[length]) {
// 		std::copy_n(str, length, data);
// 	}

// 	operator const char*() const { return data; }

// 	char data[length];
// 	auto operator<=>(const StringLiteral&) const = default;
// 	bool operator==(const StringLiteral&) const = default;
// };

// template <typename Type, StringLiteral name, StringLiteral description, typename ParentType = Driver>
// struct IsDriverType: ParentType {
// 	typedef ParentType Super;

// 	static inline DriverType typeInstance{name, description};

// 	/**/ IsDriverType(Startup startup):
// 		Super(startup)
// 	{
// 		typeInstance.parentType = Driver::type;
// 		Driver::type = &typeInstance;
// 	}
// };

// template <typename Type, StringLiteral name, StringLiteral description, typename ParentType = Driver>
// struct IsDriverInstance: ParentType {
// 	typedef ParentType Super;

// 	static inline DriverType typeInstance{name, description};

// 	static Type instance;

// 	/**/ IsDriverInstance(Startup startup):
// 		Super(startup)
// 	{
// 		typeInstance.parentType = Driver::type;
// 		Driver::type = &typeInstance;
// 	}

// 	// auto get_priority() -> U32 override;
// };
