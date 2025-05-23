#include "DriverApi.hpp"

#include <drivers/Interrupt.hpp>

#include <kernel/arch/x86/PciDevice.hpp>
#include <kernel/Driver.hpp>
#include <kernel/DriverReference.hpp>
#include <kernel/drivers.hpp>

#include <common/PodArray.hpp>

#include <cstddef>

namespace drivers {
	extern PodArray<Driver*> *interruptSubscribers[256];
	extern PodArray<Driver*> *irqSubscribers[256];
}

auto DriverApi::driver() -> Driver& {
	return *(Driver*)((char*)(this)-offsetof(Driver, api));
}

void DriverApi::subscribe_interrupt(U8 vector) {
	if(subscribedInterrupts.get(vector)) return;

	auto &driver = this->driver();

	auto &subscribers = drivers::interruptSubscribers[vector];
	if(!subscribers){
		subscribers = new PodArray<Driver*>(1);
	}
	subscribers->push_back(&driver);
	subscribedInterrupts.set(vector, true);
}

void DriverApi::unsubscribe_interrupt(U8 vector) {
	if(!subscribedInterrupts.get(vector)) return;

	auto &driver = this->driver();

	auto &subscribers = drivers::interruptSubscribers[vector];
	if(subscribers){
		for(auto i=0u;i<subscribers->length;i++){
			if((*subscribers)[i]==&driver){
				subscribers->remove(i);
				break;
			}
		}
	}

	subscribedInterrupts.set(vector, false);
}

void DriverApi::unsubscribe_all_interrupts() {
	for(auto vector=0u;vector<256;vector++){
		if(subscribedInterrupts.get(vector)){
			unsubscribe_interrupt(vector);
		}
	}
}

auto DriverApi::subscribe_irq(U8 irq) -> Try<> {
	if(subscribedIrqs.get(irq)) return {};

	TRY(drivers::_subscribe_driver_to_irq(this->driver(), irq));

	subscribedIrqs.set(irq, true);

	return {};
}

auto DriverApi::subscribe_available_irq(Bitmask256 bitmask) -> Try<U8> {
	drivers::find_and_activate<driver::Interrupt>(&driver()); //ensure at least 1 is active

	for(auto &driver:drivers::iterate<driver::Interrupt>()){
		auto irqRequest = driver.get_available_irq(bitmask);
		if(!irqRequest) continue;

		const auto irq = irqRequest.result;

		if(!drivers::_subscribe_driver_to_irq(this->driver(), irqRequest.result)) continue;

		subscribedIrqs.set(irq, true);

		return {irqRequest.result};
	}

	return Failure{"No subscribable IRQs available"};
}

void DriverApi::unsubscribe_irq(U8 irq) {
	if(!subscribedIrqs.get(irq)) return;

	drivers::_unsubscribe_driver_from_irq(this->driver(), irq);

	subscribedIrqs.set(irq, false);
}

void DriverApi::unsubscribe_all_irqs() {
	for(auto vector=0u;vector<256;vector++){
		if(subscribedIrqs.get(vector)){
			unsubscribe_irq(vector);
		}
	}
}

auto DriverApi::subscribe_memory(Physical<void> start, size_t size, mmu::Caching caching) -> Try<void*> {
	auto &driver = this->driver();

	if(start<memory::heap+memory::heapSize&&start+size>memory::heap){
		return Failure{"Memory not available - Overlaps heap"};
	}

	// deny if this memory is already in use by an active driver
	for(auto &otherDriver:drivers::iterate<Driver>()){
		if(&otherDriver==&driver||!otherDriver.api.is_active()) continue;

		if(otherDriver.api.is_subscribed_to_memory(start, size)) return Failure{"Memory not available - Already in use"};
	}

	// grow existing subscriptions if it overlaps or comes before an entry
	auto end = start+size;
	for(auto i=0u;i<subscribedMemory.length;i++){
		auto &subscribed = subscribedMemory[i];

		if(subscribed.start>end){ // this region is after, so insert a new record before
			subscribedMemory.insert(i, MemoryRange{start, end});
			//FIXME: return virtual address
			return {(void*)start.address};
		}

		if(subscribed.end<start) continue; // this is before, continue

		// region overlaps

		if(start<subscribed.start){
			subscribed.start = start; // we know it didn't touch the previous subscription, so we can just expand this one to the left
		}

		if(end>subscribed.end){
			subscribed.end = end;
			while(i+1<subscribedMemory.length&&subscribedMemory[i+1].start<=end){
				subscribed.end = max(subscribed.end, subscribedMemory[i+1].end);
				subscribedMemory.remove(i+1);
			}
		}

		return mmu::kernel::transaction().map_physical_high(start, size, {.caching = caching});
	}

	// insert at the very end if it wasn't merged in prior
	subscribedMemory.push_back(MemoryRange{start, end});

	//TODO: use from an existing recycle pool of mappings instead, if available there (we say recycled as we may want to change options, and thus don't want it in use by anything else)
	auto virtualAddress = mmu::kernel::transaction().map_physical_high(start, size, {.caching = caching});

	return virtualAddress;
}

void DriverApi::unsubscribe_memory(Physical<void> start, size_t _size) {
	//TODO: move these virtual mappings into a recycle pool, so they can be claimed again by other drivers when needed

	auto end = start+_size;
	for(auto i=0u;i<subscribedMemory.length;i++){
		auto &subscribed = subscribedMemory[i];

		if(subscribed.start>=end) break;
		if(subscribed.end<=start) continue;

		// region overlaps

		if(subscribed.start>=start&&subscribed.end<=end){ // whole region covered
			subscribedMemory.remove(i--);
			continue;

		}else if(start>subscribed.start){
			auto subscribedEnd = subscribed.end;
			subscribed.end = start; // left half

			if(subscribedEnd>end){ // right half
				subscribedMemory.insert(i+1, MemoryRange{end, subscribedEnd});
			}

		}else if(end<subscribed.end){
			subscribed.start = end; // right half
		}
	}
}

void DriverApi::unsubscribe_all_memory() {
	//TODO: move these virtual mappings into a recycle pool, so they can be claimed again by other drivers when needed

	subscribedMemory.clear();
}

auto DriverApi::is_subscribed_to_memory(Physical<void> start, size_t _size) -> bool {
	auto end = start+_size;
	for(auto i=0u;i<subscribedMemory.length;i++){
		auto &subscribed = subscribedMemory[i];

		if(subscribed.start>=end) break;
		if(subscribed.end<=start) continue;

		// region overlaps

		return true;
	}

	return false;
}

auto DriverApi::subscribe_pci(PciDevice &pciDevice, PciOptions pciOptions) -> Try<> {
	if(is_subscribed_to_pci(pciDevice)) return {};

	for(auto &driver:drivers::iterate<Driver>()){
		if(driver.api.is_subscribed_to_pci(pciDevice)) return Failure{"PCI device not available - already in use"};
	}

	subscribedPciDevices.push_back(&pciDevice);
	pciDevice.enable_io_space(pciOptions.ioSpace);
	pciDevice.enable_memory_space(pciOptions.memorySpace);

	//TODO: should we be unsetting these when we let go? do we need to also disable options we don't want?
	if(pciOptions.busMastering) pciDevice.enable_bus_mastering(true);
	if(pciOptions.interrupts) pciDevice.enable_interrupts(true);

	return {};
}

void DriverApi::unsubscribe_pci(PciDevice &pciDevice) {
	for(auto i=0u;i<subscribedPciDevices.length;i++){
		if(subscribedPciDevices[i]==&pciDevice){
			subscribedPciDevices.remove(i);
			break;
		}
	}
}

void DriverApi::unsubscribe_all_pci() {
	for(auto &device:subscribedPciDevices){
		device->enable_io_space(false);
		device->enable_memory_space(false);
	}
	//TODO: reset other options?

	subscribedPciDevices.clear();
}

auto DriverApi::is_subscribed_to_pci(PciDevice &pciDevice) -> bool {
	for(auto device:subscribedPciDevices){
		if(device==&pciDevice) return true;
	}

	return false;
}

#ifdef ARCH_X86
	auto DriverApi::subscribe_ioPort(arch::x86::IoPort ioPort) -> Try<arch::x86::IoPort> {
		if(is_subscribed_to_ioPort(ioPort)) return ioPort;

		for(auto &other:drivers::iterate<Driver>()){
			if(other.api.is_subscribed_to_ioPort(ioPort)) return Failure{"I/O port not available - already in use"};
		}

		//TODO: insert ordered
		subscribedIoPorts.push_back(ioPort);

		return ioPort;
	}

	void DriverApi::unsubscribe_ioPort(arch::x86::IoPort ioPort) {
		for(auto i=0u;i<subscribedIoPorts.length;i++){
			if(subscribedIoPorts[i]==ioPort){
				subscribedIoPorts.remove(i);
				break;
			}
		}
	}

	void DriverApi::unsubscribe_all_ioPort() {
		subscribedIoPorts.clear();
	}

	auto DriverApi::is_subscribed_to_ioPort(arch::x86::IoPort ioPort) -> bool {
		for(auto subbed:subscribedIoPorts){
			if(subbed==ioPort) return true;
		}

		return false;
	}
#endif

auto DriverApi::start_driver() -> Try<> {
	if(state==State::active) return {};

	if(auto result = driver()._on_start(); !result){
		state = State::failed;

		unsubscribe_all_memory();
		unsubscribe_all_interrupts();
		unsubscribe_all_irqs();
		unsubscribe_all_pci();
		unsubscribe_all_ioPort();

		return result;
	}

	state = State::active;
	return {};
}

auto DriverApi::stop_driver() -> Try<> {
	if(state!=State::active) return {};

	TRY(driver()._on_stop());
	state = State::inactive;

	unsubscribe_all_memory();
	unsubscribe_all_interrupts();
	unsubscribe_all_irqs();
	unsubscribe_all_pci();
	unsubscribe_all_ioPort();

	for(auto reference = driver().references.head; reference; reference=reference->next) {
		reference->terminate();
	}

	driver().references.clear();

	return {};
}

auto DriverApi::restart_driver() -> Try<> {
	TRY(stop_driver());

	if(state==State::active) return {};

	TRY(start_driver());

	return {};
}

void DriverApi::fail_driver(const char *reason) {
	//TODO: log this reason, and store it in the driver api for later query

	state = State::failed;

	unsubscribe_all_memory();
	unsubscribe_all_interrupts();
	unsubscribe_all_irqs();
	unsubscribe_all_pci();
	unsubscribe_all_ioPort();
}
