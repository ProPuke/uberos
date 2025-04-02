#include "PciDevice.hpp"

void PciDevice::enable_io_space(bool enable) {
	if(enable){
		writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) | (1<<0));
	}else{
		writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) & ~(1<<0));
	}
}

void PciDevice::enable_memory_space(bool enable) {
	if(enable){
		writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) | (1<<1));
	}else{
		writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) & ~(1<<1));
	}
}

void PciDevice::enable_bus_mastering(bool enable) {
	if(enable){
		writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) | (1<<2));
	}else{
		writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) & ~(1<<2));
	}
}

void PciDevice::enable_interrupts(bool enable) {
	if(enable){
		writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) & ~(1<<10));
	}else{
		writeConfig16((UPtr)RegisterOffset::command, readConfig16((UPtr)RegisterOffset::command) | (1<<10));
	}
}
