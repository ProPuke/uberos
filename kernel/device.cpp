#include "device.hpp"

#include <common/format.hpp>

#include <kernel/Driver.hpp>
#include <kernel/driver/Graphics.hpp>
#include <kernel/driver/Processor.hpp>
#include <kernel/driver/Serial.hpp>
#include <kernel/stdio.hpp>

namespace device {
	LList<Driver> devices;

	void install_device(Driver &device, bool enable) {
		devices.push_back(device);
		if(enable){
			start_device(device);
		}
	}

	auto start_device(Driver &device) -> bool {
		if(device.state==Driver::State::enabled||device.state==Driver::State::restarting) return true;

		stdio::Section section("device::", device.name, "::start");

		Driver *disabledDevice = nullptr;

		// stop if there's an existing driver that would conflict (or fail if we can't)
		if(device.address){
			for(Driver &existing:iterate_all<Driver>()){
				if(&existing==&device||existing.state==Driver::State::disabled) continue;

				if(existing.address==device.address){ //check for any other conflict scenarios here
					if(!stop_device(existing)) return false;
					disabledDevice = &existing;
				}
			}
		}

		device._on_driver_enable();
		if(device.state!=Driver::State::enabled){
			stdio::print_error("Failed to start device");
			//if we failed to start the driver, and stopped a previous in order to try, then restore it
			if(disabledDevice){
				start_device(*disabledDevice);
			}
			return false;
		}

		return true;
	}

	auto stop_device(Driver &device) -> bool {
		if(device.state==Driver::State::disabled) return true;
		if(!device.can_disable_driver()) return false;

		stdio::Section section("device::", device.name, "::stop");

		device._on_driver_disable();
		if(device.state!=Driver::State::disabled){
			stdio::print_error("Failed to stop device");
			return false;
		}

		return true;
	}

	auto restart_device(Driver &device) -> bool {
		if(device.state==Driver::State::restarting) return true;
		if(!device.can_restart_driver()) return false;

		stdio::Section section("device::", device.name, "::restart");

		if(device.state==Driver::State::disabled) return start_device(device);

		device._on_driver_restart();
		if(device.state==Driver::State::disabled){
			stdio::print_error("Failed to restart device");
			return false;
		}

		return true;
	}

	auto find_first() -> Driver* {
		return devices.head;
	}
	auto find_first_type(const char *type) -> Driver* {
		for(Driver *device = devices.head; device; device = device->next){
			if(!strcmp(device->type, type)) return device;
		}

		return nullptr;
	}
	auto find_next_type(Driver &after, const char *type) -> Driver* {
		for(Driver *device = after.next; device; device = device->next){
			if(!strcmp(device->type, type)) return device;
		}

		return nullptr;
	}

	void print_summary() {
		auto first = true;
		for(auto device = devices.head; device; device=device->next){
			if(!first) stdio::print_info();
			
			stdio::print_info_start();
			switch(device->state){
				case Driver::State::disabled:
					stdio::print_inline("[-] ");
				break;
				case Driver::State::enabled:
					stdio::print_inline("[+] ");
				break;
				case Driver::State::failed:
					stdio::print_inline("[!] ");
				break;
				case Driver::State::restarting:
					stdio::print_inline("[?] ");
				break;
			}
			stdio::print_inline(device->name);
			if(device->descriptiveType){
				stdio::print_inline(" - ", device->descriptiveType);
			}
			// if(device->address){
			// 	stdio::print_inline(" @ ", format::Hex64{device->address});
			// }
			stdio::print_end();
			// stdio::print_info("   State: ", device->state);
			if(device->address){
				stdio::print_info("   Address: ", format::Hex64{device->address});
			}
			if(!strcmp(device->type, "processor")){
				auto &processor = *(driver::Processor*)device;
				stdio::print_info("   Architecture: ", processor.processor_arch);
				stdio::print_info("   Cores: ", processor.processor_cores);

				// for(U32 i=0;i<processor.get_voltage_count();i++){
				// 	stdio::print_info("   ", processor.get_voltage_name(i), " = ", processor.get_voltage_value(i), " V");
				// }

				// for(U32 i=0;i<processor.get_clock_count();i++){
				// 	stdio::print_info("   ", processor.get_clock_name(i), " = ", processor.get_clock_value(i)/1000000.0f, " Mhz");
				// }

				// for(U32 i=0;i<processor.get_temperature_count();i++){
				// 	stdio::print_info("   ", processor.get_temperature_name(i), " = ", processor.get_temperature_value(i), " K");
				// }
			}
			if(!strcmp(device->type, "serial")){
				auto &serial = *(driver::Serial*)device;
				if(serial.state==driver::Serial::State::enabled){
					stdio::print_info("   Baud: ", serial.get_active_baud());
				}
			}
			if(!strcmp(device->type, "graphics")){
				auto &graphics = *(driver::Graphics*)device;
				stdio::print_info("   Framebuffers: ", graphics.get_framebuffer_count());
				auto defaultMode = graphics.get_default_mode();
				if(defaultMode.width){
					stdio::print_info("   Default mode: ", defaultMode.width, "x", defaultMode.height, " @ ", defaultMode.format);
				}else{
					stdio::print_info("   Default mode: None");
				}
			}

			first = false;
		}
	}
}
