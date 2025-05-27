#pragma once

#include <drivers/Software.hpp>

//FIXME: this seems like an ugly hack. Drivers do currently depend on some driver services never stopping or being replaced though, so this seems the best solution for now
// ideally we would allow this though (otherwise we need to reboot to replace/update these, which sucks)

namespace driver {
	template <typename Type>
	struct ResidentService: Type {
		DRIVER_TYPE(ResidentService, 0xdc28cbbc, "resident", "Resident service", Type)

		auto can_stop_driver() -> bool override { return false; }
		auto can_restart_driver() -> bool override { return false; }

		auto _on_stop() -> Try<> override { return Failure{"A resident service driver cannot be stopped"}; };
	};
}
