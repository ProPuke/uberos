#pragma once

#include <kernel/DriverApi.hpp>
#include <kernel/drivers.hpp>
#include <kernel/memory.hpp>

#include <common/Bool256.hpp>
#include <common/LList.hpp>

struct DriverType {
	const char *name;
	DriverType *parentType;
};

struct Driver: LListItem<Driver> {
	static DriverType driverType;

	const char *name;
	DriverType *type;
	DriverApi api;
	const char *description;

	/*   */ /**/ Driver(const char *name, const char *description);
	virtual /**/~Driver();

	auto is_type(DriverType &compare) -> bool {
		for(auto type=this->type;type;type=type->parentType){
			if(type==&compare) return true;
		}
		return false;
	}

	virtual auto can_disable_driver() -> bool { return true; }
	virtual auto can_restart_driver() -> bool { return true; }

protected:

	friend class DriverApi;
	friend void drivers::_on_irq(U8);
	friend auto drivers::_on_interrupt(U8 vector, const void *cpuState) -> const void*;

	virtual auto _on_start() -> bool = 0;
	virtual auto _on_stop() -> bool = 0;

	virtual auto _on_interrupt(U8, const void *cpuState) -> const void* { return nullptr; }
	virtual void _on_irq(U8) {}
};

