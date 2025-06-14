#pragma once

#include <drivers/Hardware.hpp>

#include <common/Try.hpp>

namespace driver {
	struct StorageController: Hardware {
		DRIVER_TYPE(StorageController, 0x83f29031, "storageController", "Storage controller", Hardware);

		virtual auto get_drive_count() -> U32 = 0;
		virtual auto get_drive_id(U32) -> U32 = 0;

		virtual auto does_drive_exist(U32) -> bool { return false; }
		virtual auto get_drive_name(U32) -> Try<const char*> = 0;
		virtual auto get_drive_description(U32) -> Try<const char*> = 0;
		virtual auto get_drive_size(U32) -> Try<U64> = 0;
		virtual auto get_drive_model(U32) -> Try<const char*> = 0;
		virtual auto get_drive_serialNumber(U32) -> Try<const char*> = 0;
		virtual auto is_drive_present(U32) -> Try<bool> = 0;
		virtual auto is_drive_removable(U32) -> Try<bool> = 0;
		virtual auto eject_drive(U32) -> Try<bool> = 0;
	};
}
