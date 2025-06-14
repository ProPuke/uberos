#include "StorageManager.hpp"

#include <drivers/StorageController.hpp>

#include <common/String.hpp>
#include <common/ListOrdered.hpp>
#include <common/ListUnordered.hpp>
#include <common/PodArray.hpp>

namespace driver {
	namespace {
		struct DriveName {
			String8 prefix;
			U32 count = 0;
		};

		struct Drive {
			String8 name;
			StorageController *controller;
			U32 index;
			bool isSystemMapping = false;
		};

		PodArray<DriveName> driveNames;
		PodArray<Drive> drives;

		Lock<LockType::flat> lock;
	}

	auto StorageManager::_on_start() -> Try<> {
		return {};
	}

	auto StorageManager::get_drive_count() -> U32 {
		Lock_Guard guard(lock);

		return drives.length;
	}

	auto StorageManager::find_drive_by_name(const char *name) -> Try<U32> {
		Lock_Guard guard(lock);

		for(auto i=0u;i<drives.length;i++){
			if(drives[i].name==name) return i;
		}

		return Failure{"drive not found"};
	}

	auto StorageManager::does_drive_exist(U32 index) -> bool {
		Lock_Guard guard(lock);

		if(index>=drives.length) return false;

		auto &drive = drives[index];

		if(!drive.controller) return false;

		return drive.controller->does_drive_exist(drive.index);
	}

	auto StorageManager::get_drive_name(U32 index) -> Try<const char*> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		return drives[index].name.get_data();
	}

	auto StorageManager::get_drive_description(U32 index) -> Try<const char*> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) Failure{"drive not present"};

		return drive.controller->get_drive_description(drive.index);
	}

	auto StorageManager::get_drive_size(U32 index) -> Try<U64> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) Failure{"drive not present"};

		return drive.controller->get_drive_size(drive.index);
	}

	auto StorageManager::get_drive_model(U32 index) -> Try<const char*> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) Failure{"drive not present"};

		return drive.controller->get_drive_model(drive.index);
	}

	auto StorageManager::get_drive_serialNumber(U32 index) -> Try<const char*> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) Failure{"drive not present"};

		return drive.controller->get_drive_serialNumber(drive.index);
	}

	auto StorageManager::get_drive_controller(U32 index) -> Try<StorageController*> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) Failure{"drive not present"};

		return drive.controller;
	}

	auto StorageManager::get_drive_controller_index(U32 index) -> Try<U32> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) Failure{"drive not present"};

		return drive.index;
	}

	auto StorageManager::get_drive_systemId(U32 index) -> Try<U32> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) Failure{"drive not present"};

		for(auto i=0u;i<drives.length;i++){
			auto &existingDrive = drives[i];
			if(!existingDrive.isSystemMapping) continue;
			if(existingDrive.controller==drive.controller&&existingDrive.index==drive.index) return i;
		}

		return Failure{"system drive not found"};
	}

	auto StorageManager::is_drive_present(U32 index) -> Try<bool> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) return Failure{"drive not present"};

		return drive.controller->is_drive_present(drive.index);
	}

	auto StorageManager::is_drive_removable(U32 index) -> Try<bool> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) Failure{"drive not present"};

		return drive.controller->is_drive_removable(drive.index);
	}

	auto StorageManager::is_system_drive(U32 index) -> bool {
		Lock_Guard guard(lock);

		if(index>=drives.length) return false;

		auto &drive = drives[index];
		return drive.isSystemMapping;
	}

	auto StorageManager::eject_drive(U32 index) -> Try<bool> {
		Lock_Guard guard(lock);

		if(index>=drives.length) return Failure{"drive not present"};

		auto &drive = drives[index];
		if(!drive.controller) Failure{"drive not present"};

		return drive.controller->eject_drive(drive.index);
	}

	auto StorageManager::allocate_name(AllocationOptions options, StorageController &controller, U32 index) -> const char* {
		Lock_Guard guard(lock);

		DriveName *driveName;

		for(auto &existing:driveNames){
			if(existing.prefix == options.prefix){
				driveName = &existing;
				goto existingFound;
			}
		}

		driveName = &driveNames.push_back(options.prefix);

		existingFound:

		auto id = driveName->count++;
		auto &newDrive = drives.push_back(options.prefix, &controller, index);
		newDrive.isSystemMapping = options.systemMapping;
		if(id>0||!options.singular){
			newDrive.name.append(to_string(id+1));
		}

		return newDrive.name.get_data();
	}

	// once released we do not ever reuse previous names - they just remain null
	// we don't want an old drive name ever be assigned to a new different drive, as this could cause actions being accidentally applied to other drives
	// TODO: when restarting StorageController drivers, avoid releasing and re-allocating drives so that they can continue with the same drive names (if they're still the same drive with i.e. the same serial number)
	void StorageManager::release_name(const char *name) {
		Lock_Guard guard(lock);

		for(auto &drive:drives){
			if(drive.name==name){
				drive.controller = nullptr;
				break;
			}
		}
	}

	auto StorageManager::set_allocation_id(const char *name, U32 driveId) -> Try<> {
		Lock_Guard guard(lock);

		for(auto i=0u;i<drives.length;i++){
			if(drives[i].name==name) {
				drives[i].index = driveId;
				return {};
			}
		}

		return Failure{"allocated drive not found"};
	}
}
