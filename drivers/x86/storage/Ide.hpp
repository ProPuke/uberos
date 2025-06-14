#pragma once

#include <drivers/StorageController.hpp>

namespace driver::storage {
	struct Ide final: driver::StorageController {
		DRIVER_INSTANCE(Ide, 0x675f2721, "ide", "IDE ATA/ATAPI controller", driver::StorageController)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto get_drive_count() -> U32 override;
		auto get_drive_id(U32) -> U32 override;

		auto does_drive_exist(U32) -> bool override;
		auto get_drive_name(U32) -> Try<const char*> override;
		auto get_drive_description(U32) -> Try<const char*> override;
		auto get_drive_model(U32) -> Try<const char*> override;
		auto get_drive_serialNumber(U32) -> Try<const char*> override;
		auto get_drive_size(U32) -> Try<U64> override;
		auto is_drive_present(U32) -> Try<bool> override;
		auto is_drive_removable(U32) -> Try<bool> override;
		auto eject_drive(U32) -> Try<bool> override;
	};
}
