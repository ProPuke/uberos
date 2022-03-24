#pragma once

#include <common/LList.hpp>

#include <kernel/device.hpp>
#include <kernel/memory.hpp>

struct Driver: LListItem<Driver> {
	enum struct State {
		disabled,
		enabled,
		restarting,
		failed,
		max = failed
	};
	static const char * state_name[(U64)State::max+1];

	U64 address;
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

	virtual auto can_disable_driver() -> bool { return true; }
	virtual auto can_restart_driver() -> bool { return true; }

private:

	friend auto device::start_device(Driver &device) -> bool;
	friend auto device::stop_device(Driver &device) -> bool;
	friend auto device::restart_device(Driver &device) -> bool;

	virtual void _on_driver_enable() { state = State::enabled; };
	virtual void _on_driver_disable() { state = State::disabled; };
	virtual void _on_driver_restart() { _on_driver_disable(); _on_driver_enable(); };
};

inline auto to_string(Driver::State state) -> const char* { return Driver::state_name[(U64)state]; }
