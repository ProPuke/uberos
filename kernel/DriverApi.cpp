#include "DriverApi.hpp"

#include "Driver.hpp"

#include <kernel/PodArray.hpp>

#include <cstddef>

namespace drivers {
	extern PodArray<Driver*> *interruptSubscribers[256];
	extern PodArray<Driver*> *irqSubscribers[256];
}

const char * DriverApi::state_name[(U64)DriverApi::State::max+1] = {
	"disabled",
	"enabled",
	"restarting",
	"failed"
};

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

void DriverApi::subscribe_irq(U8 irq) {
	if(subscribedIrqs.get(irq)) return;

	auto &driver = this->driver();

	auto &subscribers = drivers::irqSubscribers[irq];
	if(!subscribers){
		subscribers = new PodArray<Driver*>(1);
	}
	subscribers->push_back(&driver);
	subscribedIrqs.set(irq, true);
}

void DriverApi::unsubscribe_irq(U8 irq) {
	if(!subscribedIrqs.get(irq)) return;

	auto &driver = this->driver();

	auto &subscribers = drivers::irqSubscribers[irq];
	if(subscribers){
		for(auto i=0u;i<subscribers->length;i++){
			if((*subscribers)[i]==&driver){
				subscribers->remove(i);
				break;
			}
		}
	}

	subscribedIrqs.set(irq, false);
}

void DriverApi::unsubscribe_all_irqs() {
	for(auto vector=0u;vector<256;vector++){
		if(subscribedIrqs.get(vector)){
			unsubscribe_irq(vector);
		}
	}
}

auto DriverApi::subscribe_memory(void *start, size_t _size) -> bool {
	auto &driver = this->driver();

	// deny if this memory is already in use by an active driver
	for(auto &otherDriver:drivers::iterate<Driver>()){
		if(&otherDriver==&driver||!otherDriver.api.is_active()) continue;

		if(otherDriver.api.is_subscribed_to_memory(start, _size)) return false;
	}

	// grow existing subscriptions if it overlaps or comes before an entry
	void *end = (U8*)start+_size;
	for(auto i=0u;i<subscribedMemory.length;i++){
		auto &subscribed = subscribedMemory[i];

		if(subscribed.start>end){ // this region is after, so insert a new record before
			subscribedMemory.insert(i, MemoryRange{start, end});
			return true;
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

		return true;
	}

	// insert at the very end if it wasn't merged in prior
	subscribedMemory.push_back(MemoryRange{start, end});

	return true;
}

void DriverApi::unsubscribe_memory(void *start, size_t _size) {
	void *end = (U8*)start+_size;
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
	subscribedMemory.clear();
}

auto DriverApi::is_subscribed_to_memory(void *start, size_t _size) -> bool {
	void *end = (U8*)start+_size;
	for(auto i=0u;i<subscribedMemory.length;i++){
		auto &subscribed = subscribedMemory[i];

		if(subscribed.start>=end) break;
		if(subscribed.end<=start) continue;

		// region overlaps

		return true;
	}

	return false;
}

void DriverApi::enable_driver() {
	if(state==State::enabled||state==State::active) return;

	state = State::enabled;
}

void DriverApi::start_driver() {
	if(state==State::active) return;

	if(!driver()._on_start()){
		state = State::failed;
		return;
	}

	state = State::active;
}

void DriverApi::stop_driver() {
	if(state!=State::active) return;

	if(!driver()._on_stop()) return;
	state = State::disabled;

	unsubscribe_all_memory();
	unsubscribe_all_interrupts();
	unsubscribe_all_irqs();
}

void DriverApi::restart_driver() {
	stop_driver();

	if(state==State::active) return;

	start_driver();
}
