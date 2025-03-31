#include "PciDevice.hpp"

void PciDevice::enable_io_space() {
	writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) | (1<<0));
}
void PciDevice::disable_io_space() {
	writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) & ~(1<<0));
}

void PciDevice::enable_memory_space() {
	writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) | (1<<1));
}
void PciDevice::disable_memory_space() {
	writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) & ~(1<<1));
}

void PciDevice::enable_bus_mastering() {
	writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) | (1<<2));
}
void PciDevice::disable_bus_mastering() {
	writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) & ~(1<<2));
}

void PciDevice::enable_interrupts() {
	writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) & ~(1<<10));
}
void PciDevice::disable_interrupts() {
	writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) | (1<<10));
}
