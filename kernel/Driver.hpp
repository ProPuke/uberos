#pragma once

#include <common/types.hpp>
#include <common/LList.hpp>

#include <kernel/memory.hpp>

struct Driver: LListItem<Driver> {
	enum struct State {
		disabled,
		enabled,
		enabling,
		disabling,
		restarting,
		failed,
		max = failed
	};
	static const char * state_name[(U64)State::max+1];

	U64 address;
	bool is_builtin = false;
	const char *name;
	const char *type;
	const char *descriptiveType;
	State state = State::disabled;

	//NOTE:constexpr helps ensure that drivers are (hopefully) valid from the start, and not invalid and then overwritten later by __init_array_start
	constexpr /**/ Driver(U64 address, const char *name, const char *type, const char *descriptiveType):
		address(address),
		name(name),
		type(type),
		descriptiveType(descriptiveType)
	{}

	virtual /**/~Driver();

	auto get_driver_state() -> State { return State::enabled; };

	virtual auto can_disable_driver() -> bool { return !is_builtin; }
	virtual auto can_restart_driver() -> bool { return true; }

	virtual void enable_driver() { state = State::enabled; };
	virtual void disable_driver() { state = State::disabled; };
	virtual void restart_driver() { disable_driver(); enable_driver(); };
};

inline auto to_string(Driver::State state) -> const char* { return Driver::state_name[(U64)state]; }
