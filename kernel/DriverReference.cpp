#include "DriverReference.hpp"

#include <kernel/Driver.hpp>

/**/ DriverReference<Driver>:: DriverReference(const DriverReference &copy):
	driver(copy.driver),
	onTerminated(copy.onTerminated),
	onTerminatedData(copy.onTerminatedData)
{
	if(driver){
		driver->references.pop(*this);
	}
}

/**/ DriverReference<Driver>:: DriverReference(Driver *driver, Callback onTerminated, void *onTerminatedData):
	driver(driver),
	onTerminated(onTerminated),
	onTerminatedData(onTerminatedData)
{
	if(driver){
		driver->references.push_back(*this);
	}
}

/**/ DriverReference<Driver>::~DriverReference() {
	if(driver){
		driver->references.pop(*this);
	}
}

auto DriverReference<Driver>::operator=(const DriverReference &copy) -> DriverReference& {
	if(driver!=copy.driver){
		if(driver){
			driver->references.pop(*this);
		}

		driver = copy.driver;

		if(driver){
			driver->references.push_back(*this);
		}
	}

	onTerminated = copy.onTerminated;
	onTerminatedData = copy.onTerminatedData;

	return *this;
}

auto DriverReference<Driver>::operator=(Driver *set) -> DriverReference& {
	if(set==driver) return *this;

	if(driver){
		driver->references.pop(*this);
	}

	driver = set;
	if(driver){
		driver->references.push_back(*this);
	}

	return *this;
}

void DriverReference<Driver>::terminate() {
	if(!driver) return;

	driver = nullptr;
	onTerminated(onTerminatedData);
}
