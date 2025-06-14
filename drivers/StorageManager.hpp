#pragma once

#include <drivers/ResidentService.hpp>
#include <drivers/Software.hpp>

#include <common/Try.hpp>

namespace driver {
	struct StorageController;

	struct StorageManager: ResidentService<Software> {
		DRIVER_INSTANCE(StorageManager, 0x2c9497e1, "storage", "Storage manager", ResidentService<Software>);

		auto _on_start() -> Try<> override;

		auto get_drive_count() -> U32;
		auto find_drive_by_name(const char*) -> Try<U32>;

		auto does_drive_exist(U32) -> bool;
		auto get_drive_name(U32) -> Try<const char*>;
		auto get_drive_description(U32) -> Try<const char*>;
		auto get_drive_size(U32) -> Try<U64>;
		auto get_drive_model(U32) -> Try<const char*>;
		auto get_drive_serialNumber(U32) -> Try<const char*>;
		auto get_drive_systemId(U32) -> Try<U32>;
		auto get_drive_controller(U32 index) -> Try<StorageController*>;
		auto get_drive_controller_index(U32 index) -> Try<U32>;
		auto is_drive_present(U32) -> Try<bool>;
		auto is_drive_removable(U32) -> Try<bool>;
		auto is_system_drive(U32) -> bool;
		auto eject_drive(U32) -> Try<bool>;

		struct AllocationOptions {
			const char *prefix;
			bool systemMapping = false; // refers to the system device. Hidden by default
			bool singular = false; // prefer singular naming (don't put a number on the end for item 1)
		};

		auto allocate_name(AllocationOptions options, StorageController&, U32 driveId = (U32)~0) -> const char*;
		void release_name(const char*);
		auto set_allocation_id(const char*, U32 driveId) -> Try<>;
	};
}
