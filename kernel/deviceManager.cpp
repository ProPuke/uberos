#include "deviceManager.hpp"

#include <common/format.hpp>

#include <kernel/Driver.hpp>
#include <kernel/driver/Graphics.hpp>
#include <kernel/driver/Processor.hpp>
#include <kernel/driver/Serial.hpp>
#include <kernel/stdio.hpp>

namespace deviceManager {
	LList<Driver> devices;

	void add_device(Driver &device, bool enable) {
		devices.push_back(device);
		if(enable){
			device.enable_driver();
		}
	}

	void print_summary() {
		auto first = true;
		for(auto device = devices.head; device; device=device->next){
			if(!first) stdio::print_info();
			
			stdio::print_info_start();
			stdio::print_inline(device->name);
			if(device->descriptiveType){
				stdio::print_inline(" - ", device->descriptiveType);
			}
			// if(device->address){
			// 	stdio::print_inline(" @ ", format::Hex64{device->address});
			// }
			stdio::print_end();
			stdio::print_info("  State: ", device->state);
			if(device->address){
				stdio::print_info("  Address: ", format::Hex64{device->address});
			}
			stdio::print_info("  Location: ", device->is_builtin?"Internal":"Runtime");
			if(!strcmp(device->type, "processor")){
				auto &processor = *(driver::Processor*)device;
				stdio::print_info("  Architecture: ", processor.processor_arch);
				stdio::print_info("  Cores: ", processor.processor_cores);

				// for(U32 i=0;i<processor.get_voltage_count();i++){
				// 	stdio::print_info("  ", processor.get_voltage_name(i), " = ", processor.get_voltage_value(i), " V");
				// }

				// for(U32 i=0;i<processor.get_clock_count();i++){
				// 	stdio::print_info("  ", processor.get_clock_name(i), " = ", processor.get_clock_value(i)/1000000.0f, " Mhz");
				// }

				// for(U32 i=0;i<processor.get_temperature_count();i++){
				// 	stdio::print_info("  ", processor.get_temperature_name(i), " = ", processor.get_temperature_value(i), " K");
				// }
			}
			if(!strcmp(device->type, "serial")){
				auto &serial = *(driver::Serial*)device;
				if(serial.state==driver::Serial::State::enabled){
					stdio::print_info("  Baud: ", serial.get_active_baud());
				}
			}
			if(!strcmp(device->type, "graphics")){
				auto &graphics = *(driver::Graphics*)device;
				stdio::print_info("  Framebuffers: ", graphics.get_framebuffer_count());
				auto defaultMode = graphics.get_default_mode();
				if(defaultMode.width){
					stdio::print_info("  Default mode: ", defaultMode.width, "x", defaultMode.height, " @ ", defaultMode.format);
				}else{
					stdio::print_info("  Default mode: None");
				}
			}

			first = false;
		}
	}
}
