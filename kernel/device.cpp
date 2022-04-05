#include "device.hpp"

#include <common/format.hpp>
#include <common/maths.hpp>

#include <kernel/Driver.hpp>
#include <kernel/driver/Graphics.hpp>
#include <kernel/driver/Interrupt.hpp>
#include <kernel/driver/Processor.hpp>
#include <kernel/driver/Serial.hpp>
#include <kernel/stdio.hpp>

using namespace maths;

namespace device {
	LList<Driver> devices;

	namespace {
		void print_percent_graph(const char *indent, const char *indent2, F32 temp, F32 min, F32 max, F32 safetyMax = -1) {
			float phase = (temp-min)/(max-min);

			const char empty[] = "..............................";
			const char full[]  = "||||||||||||||||||||||||||||||";
			const U32 bars = clamp(phase, 0.0, 1.0)*(sizeof(empty)-1)+0.5;

			if(temp>=0&&temp>=safetyMax){
				stdio::print_warning_start();
			}else{
				stdio::print_info_start();
			}
			stdio::print_inline(indent, indent2, "[");
			stdio::print_inline(full+30-bars);
			stdio::print_inline(empty+30-(30-bars));
			stdio::print_inline(']');
			stdio::print_end();
		}
	}

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
			for(Driver &existing:iterate_all()){
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

	void print_device_summary(const char *indent, Driver &device) {
		stdio::print_info_start();
		stdio::print_inline(indent);

		switch(device.state){
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
		stdio::print_inline(device.name);
		if(device.descriptiveType){
			stdio::print_inline(" - ", device.descriptiveType);
		}
		// if(device.address){
		// 	stdio::print_inline(" @ ", format::Hex64{device.address});
		// }
		stdio::print_end();
		// stdio::print_info("   State: ", device.state);
		if(device.address){
			stdio::print_info(indent, "   Address: ", format::Hex64{device.address});
		}
		if(!strcmp(device.type, "processor")){
			auto &processor = *(driver::Processor*)&device;
			stdio::print_info(indent, "   Architecture: ", processor.processor_arch);
			stdio::print_info(indent, "   Cores: ", processor.processor_cores);

			auto speed = processor.get_clock_value(0);
			auto temp = processor.get_temperature_value(0);

			if(speed){
				if(speed>=1000000000){
					stdio::print_info(indent, "   Speed: ", speed/1000000000.0, " Ghz");
				}else if(speed>=1000000){
					stdio::print_info(indent, "   Speed: ", speed/1000000.0, " Mhz");
				}else if(speed>=1000){
					stdio::print_info(indent, "   Speed: ", speed/1000000.0, " Khz");
				}else if(speed>=1000){
					stdio::print_info(indent, "   Speed: ", speed/1000000.0, " Hz");
				}
			}
			if(temp){
				stdio::print_info(indent, "   Temp: ", kelvin_to_celcius(temp), " C");
				auto minTemp = celcius_to_kelvin(20);
				auto maxTemp = processor.get_temperature_max(0);

				if(maxTemp&&maxTemp>minTemp){
					print_percent_graph(indent, "   ", temp, minTemp, maxTemp+10, maxTemp);
				}
			}

			// for(U32 i=0;i<processor.get_voltage_count();i++){
			// 	stdio::print_info(indent, "   ", processor.get_voltage_name(i), " = ", processor.get_voltage_value(i), " V");
			// }

			// for(U32 i=0;i<processor.get_clock_count();i++){
			// 	stdio::print_info(indent, "   ", processor.get_clock_name(i), " = ", processor.get_clock_value(i)/1000000.0f, " Mhz");
			// }

			// for(U32 i=0;i<processor.get_temperature_count();i++){
			// 	stdio::print_info(indent, "   ", processor.get_temperature_name(i), " = ", processor.get_temperature_value(i), " K");
			// }
		}
		if(!strcmp(device.type, "serial")){
			auto &serial = *(driver::Serial*)&device;
			if(serial.state==driver::Serial::State::enabled){
				stdio::print_info(indent, "   Baud: ", serial.get_active_baud());
			}
		}
		if(!strcmp(device.type, "graphics")){
			auto &graphics = *(driver::Graphics*)&device;
			stdio::print_info(indent, "   Framebuffers: ", graphics.get_framebuffer_count());
			auto defaultMode = graphics.get_default_mode();
			if(defaultMode.width){
				stdio::print_info(indent, "   Default mode: ", defaultMode.width, "x", defaultMode.height, " @ ", defaultMode.format);
			}else{
				stdio::print_info(indent, "   Default mode: None");
			}
		}
		if(!strcmp(device.type, "interrupt")){
			auto &irq = *(driver::Interrupt*)&device;
			stdio::print_info(indent, "   Interrupts: ", irq.min_irq, " - ", irq.max_irq);
		}
	}

	bool print_device_details(const char *indent, Driver &device, const char *beforeName, const char *afterName) {
		if(!strcmp(device.type, "processor")){
			auto &processor = *(driver::Processor*)&device;

			auto clocks = processor.get_clock_count();
			auto voltages = processor.get_voltage_count();
			auto temps = processor.get_temperature_count();

			for(U32 i=0;i<clocks;i++){
				auto name = processor.get_clock_name(i);
				// auto min = processor.get_clock_min(i);
				// auto max = processor.get_clock_max(i);
				auto value = processor.get_clock_value(i);
				auto changeable = processor.can_set_clock(i);
				auto active = processor.get_clock_active_value(i);

				if(!active) active = value;

				stdio::print_info_start();
					stdio::print_inline(indent, beforeName, "clock", i, afterName, ": ", name, " = ");

					if(value>=1000000000){
						stdio::print_inline(value/1000000.0, " Ghz");
					}else if(value>=1000000){
						stdio::print_inline(value/1000000.0, " Mhz");
					}else if(value>=1000){
						stdio::print_inline(value/1000.0, " Khz");
					}else{
						stdio::print_inline(value, " Hz");
					}

					if(abs(active-value)>active/100){
						stdio::print_inline(" (currently ");

						if(active>=1000000000){
							stdio::print_inline(active/1000000.0, " Ghz");
						}else if(active>=1000000){
							stdio::print_inline(active/1000000.0, " Mhz");
						}else if(active>=1000){
							stdio::print_inline(active/1000.0, " Khz");
						}else{
							stdio::print_inline(active, " Hz");
						}
						
						stdio::print_inline(")");
					}

					if(changeable){
						stdio::print_inline(" (adjustable)");
					}

				stdio::print_end();
			}

			if(temps&&voltages){
				stdio::print_info();
			}

			for(U32 i=0;i<voltages;i++){
				auto name = processor.get_voltage_name(i);
				auto min = processor.get_voltage_min(i);
				auto max = processor.get_voltage_max(i);
				auto value = processor.get_voltage_value(i);

				if(!value) continue;

				stdio::print_info_start();
				stdio::print_inline(indent, beforeName, "voltage", i, afterName, ": ", name, " = ", value, " V");
				if(max&&max!=min){
					stdio::print_inline(" (", min, " - ", max ," V)");
				}
				stdio::print_end();
				if(max&&max>min){
					print_percent_graph(indent, "  ", value, min, max+10, max);
				}
			}

			if(voltages&&clocks){
				stdio::print_info();
			}

			for(U32 i=0;i<temps;i++){
				auto name = processor.get_temperature_name(i);
				auto min = celcius_to_kelvin(20);
				auto max = processor.get_temperature_max(i);
				auto value = processor.get_temperature_value(i);

				if(!value) continue;

				stdio::print_info_start();
				stdio::print_inline(indent, beforeName, "temp", i, afterName, ": ", name, " = ", kelvin_to_celcius(value), " C");
				if(max){
					stdio::print_inline(" (", kelvin_to_celcius(max) ," C max)");
				}
				stdio::print_end();
				if(max&&max>min){
					print_percent_graph(indent, "  ", value, min, max+10, max);
				}
			}

			return clocks>0||temps>0;

		}else if(!strcmp(device.type, "graphics")){
			auto &graphics = *(driver::Graphics*)&device;

			U32 framebuffers = graphics.get_framebuffer_count();

			for(U32 i=0;i<framebuffers;i++){
				auto name = graphics.get_framebuffer_name(i);

				stdio::print_info_start();
				stdio::print_inline(indent, beforeName, "fb", i, afterName, ": ", name);
				stdio::print_end();
			}

			if(framebuffers){
				stdio::print_info();
			}

			stdio::print_info(indent, "Reported video modes:");

			U32 count = 0;
			for(U32 i=0;i<graphics.get_mode_count();i++){
				auto mode = graphics.get_mode(0, i);

				if(!mode.width) continue;

				stdio::print_info(indent, "  ", mode.width, "x", mode.height, " @ ", mode.format);
				count++;
			}

			if(!count){
				stdio::print_warning(indent, "  ", "None found");
			}
		}


		// if(!strcmp(device.type, "serial")){
		// 	auto &serial = *(driver::Serial*)&device;
		// }
		// if(!strcmp(device.type, "graphics")){
		// 	auto &graphics = *(driver::Graphics*)&device;
		// }
		// if(!strcmp(device.type, "interrupt")){
		// 	auto &interrupt = *(driver::Interrupt*)&device;
		// }

		return false;
	}

	void print_summary() {
		auto first = true;
		for(auto &device:iterate_all()){
			if(!first) stdio::print_info();
			
			print_device_summary("", device);

			first = false;
		}
	}
}
