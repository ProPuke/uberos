#include "drivers.hpp"

#include <kernel/Driver.hpp>
#include <kernel/drivers/Graphics.hpp>
#include <kernel/drivers/Interrupt.hpp>
#include <kernel/drivers/Processor.hpp>
#include <kernel/drivers/Serial.hpp>
#include <kernel/Log.hpp>
#include <kernel/PodArray.hpp>

#include <common/format.hpp>
#include <common/maths.hpp>

static Log log("drivers");

using namespace maths;

namespace drivers {
	LList<Driver> drivers;

	PodArray<Driver*> *interruptSubscribers[256] = {};
	PodArray<Driver*> *irqSubscribers[256] = {};

	namespace {
		void print_percent_graph(const char *indent, const char *indent2, F32 temp, F32 min, F32 max, F32 safetyMax = -1) {
			float phase = (temp-min)/(max-min);

			const char empty[] = "..............................";
			const char full[]  = "||||||||||||||||||||||||||||||";
			const U32 bars = clamp(phase, 0.0, 1.0)*(sizeof(empty)-1)+0.5;

			if(temp>=0&&temp>=safetyMax){
				log.print_warning_start();
			}else{
				log.print_info_start();
			}
			log.print_inline(indent, indent2, "[");
			log.print_inline(full+30-bars);
			log.print_inline(empty+30-(30-bars));
			log.print_inline(']');
			log.print_end();
		}
	}

	void install_driver(Driver &driver, bool activate) {
		drivers.push_back(driver);
		if(activate){
			activate_driver(driver);
		}
	}

	void enable_driver(Driver &driver) {
		if(driver.api.is_enabled()) return;

		auto section = log.section("enable ", driver.name);

		driver.api.enable_driver();

		if(!driver.api.is_enabled()){
			log.print_error("Failed to enable driver");
		}
	}

	auto activate_driver(Driver &driver) -> bool {
		if(driver.api.is_active()) return true;

		auto section = log.section("START ", driver.name);

		Driver *disabledDevice = nullptr;

		driver.api.start_driver();

		if(!driver.api.is_active()){
			log.print_error("Failed to activate driver");
			//if we failed to start the driver, and stopped a previous in order to try, then restore it
			if(disabledDevice){
				activate_driver(*disabledDevice);
			}
			return false;
		}

		return true;
	}

	auto stop_driver(Driver &driver) -> bool {
		if(!driver.api.is_active()) return true;
		if(!driver.can_disable_driver()) return false;

		auto section = log.section("STOP ", driver.name);

		driver.api.stop_driver();

		if(!driver.api.is_disabled()){
			//TODO: driver might have enter a failed state while stopping. Should report this in some other way, since the driver IS now technically stopped?
			log.print_error("Failed to stop driver");
			return false;
		}

		return true;
	}

	auto restart_driver(Driver &driver) -> bool {
		if(!driver.api.is_active()){
			return activate_driver(driver);
		}

		if(!driver.can_restart_driver()) return false;

		auto section = log.section("RESTART ", driver.name);

		driver.api.restart_driver();

		if(driver.api.is_active()){
			log.print_error("Failed to restart driver");
			return false;
		}

		return true;
	}

	auto find_first(DriverType &type) -> Driver* {
		for(Driver *driver = drivers.head; driver; driver = driver->next){
			if(driver->is_type(type)) return driver;
		}

		return nullptr;
	}
	auto find_next(Driver &after, DriverType &type) -> Driver* {
		for(Driver *driver = after.next; driver; driver = driver->next){
			if(driver->is_type(type)) return driver;
		}

		return nullptr;
	}

	auto is_memory_in_use(void *address, size_t size) -> bool {
		for(auto &driver:iterate<Driver>()){
			if(!driver.api.is_active()) continue;

			if(driver.api.is_subscribed_to_memory(address, size)) return true;
		}

		return false;
	}

	auto _on_interrupt(U8 vector, const void *cpuState) -> const void* {
		auto subscribers = interruptSubscribers[vector];
		if(!subscribers) return nullptr;

		for(auto &driver:*subscribers){
			if(!driver->api.is_active()) continue;
			if(auto outputState = driver->_on_interrupt(vector, cpuState)) return outputState;
		}

		return nullptr;
	}

	void _on_irq(U8 irq) {
		auto subscribers = irqSubscribers[irq];
		if(!subscribers) return;

		for(auto &driver:*subscribers){
			if(!driver->api.is_active()) continue;
			driver->_on_irq(irq);
		}
	}

	void print_driver_summary(const char *indent, Driver &driver) {
		log.print_info_start();
		log.print_inline(indent);

		if(driver.api.is_active()){
			log.print_inline("[+] ");
		}else if(driver.api.is_active()){
			log.print_inline("[.] ");
		}else if(driver.api.is_failed()){
			log.print_inline("[!] ");
		}else if(driver.api.is_disabled()){
			log.print_inline("[x] ");
		}else{
			log.print_inline("[?] ");
		}

		log.print_inline(driver.name);
		if(driver.description){
			log.print_inline(" - ", driver.description);
		}
		// if(driver.address){
		// 	log.print_inline(" @ ", to_string(driver.address));
		// }
		log.print_end();
		// log.print_info("   State: ", driver.state);
		for(auto &subscription:driver.api.subscribedMemory){
			log.print_info(indent, "   Memory: ", to_string(subscription.start), " - ", to_string(subscription.end));
		}
		if(driver.api.subscribedIrqs.has_any()){
			log.print_info_start();
			log.print_inline(indent, "   IRQs:");
			for(auto i=0u;i<256;i++){
				if(driver.api.subscribedIrqs.get(i)){
					log.print_inline(' ', i);
				}
			}
		}
		if(driver.api.subscribedInterrupts.has_any()){
			log.print_info_start();
			log.print_inline(indent, "   Interrupts:");
			for(auto i=0u;i<256;i++){
				if(driver.api.subscribedInterrupts.get(i)){
					log.print_inline(' ', i);
				}
			}
		}
		if(driver.is_type(driver::Processor::driverType)){
			auto &processor = *(driver::Processor*)&driver;
			log.print_info(indent, "   Architecture: ", processor.processor_arch);
			log.print_info(indent, "   Cores: ", processor.processor_cores);

			auto speed = processor.get_clock_value(0);
			auto temp = processor.get_temperature_value(0);

			if(speed){
				if(speed>=1000000000){
					log.print_info(indent, "   Speed: ", speed/1000000000.0, " Ghz");
				}else if(speed>=1000000){
					log.print_info(indent, "   Speed: ", speed/1000000.0, " Mhz");
				}else if(speed>=1000){
					log.print_info(indent, "   Speed: ", speed/1000.0, " Khz");
				}else{
					log.print_info(indent, "   Speed: ", speed, " Hz");
				}
			}
			if(temp){
				log.print_info(indent, "   Temp: ", kelvin_to_celcius(temp), " C");
				auto minTemp = celcius_to_kelvin(20);
				auto maxTemp = processor.get_temperature_max(0);

				if(maxTemp&&maxTemp>minTemp){
					print_percent_graph(indent, "   ", temp, minTemp, maxTemp+10, maxTemp);
				}
			}

			// for(U32 i=0;i<processor.get_voltage_count();i++){
			// 	log.print_info(indent, "   ", processor.get_voltage_name(i), " = ", processor.get_voltage_value(i), " V");
			// }

			// for(U32 i=0;i<processor.get_clock_count();i++){
			// 	log.print_info(indent, "   ", processor.get_clock_name(i), " = ", processor.get_clock_value(i)/1000000.0f, " Mhz");
			// }

			// for(U32 i=0;i<processor.get_temperature_count();i++){
			// 	log.print_info(indent, "   ", processor.get_temperature_name(i), " = ", processor.get_temperature_value(i), " K");
			// }
		}
		if(driver.is_type(driver::Serial::driverType)){
			auto &serial = *(driver::Serial*)&driver;
			if(serial.api.is_disabled()){
				log.print_info(indent, "   Baud: ", serial.get_active_baud());
			}
		}
		if(driver.is_type(driver::Graphics::driverType)){
			auto &graphics = *(driver::Graphics*)&driver;
			const auto framebufferCount = graphics.get_framebuffer_count();
			log.print_info(indent, "   Framebuffers: ", framebufferCount);
			for(auto i=0u;i<framebufferCount;i++){
				auto defaultMode = graphics.get_default_mode(i);

				if(defaultMode.width){
					log.print_info(indent, "    ", graphics.get_framebuffer_name(i), " - Default mode: ", defaultMode.width, "x", defaultMode.height, " @ ", defaultMode.format);
				}else{
					log.print_info(indent, "    ", graphics.get_framebuffer_name(i), " - Default mode: None");
				}
			}
		}
		if(driver.is_type(driver::Interrupt::driverType)){
			auto &irq = *(driver::Interrupt*)&driver;
			log.print_info(indent, "   Interrupts: ", irq.min_irq, " - ", irq.max_irq);
		}
	}

	bool print_driver_details(const char *indent, Driver &driver, const char *beforeName, const char *afterName) {
		if(driver.is_type(driver::Processor::driverType)){
			auto &processor = *(driver::Processor*)&driver;

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

				log.print_info_start();
					log.print_inline(indent, beforeName, "clock", i, afterName, ": ", name, " = ");

					if(value>=1000000000){
						log.print_inline(value/1000000.0, " Ghz");
					}else if(value>=1000000){
						log.print_inline(value/1000000.0, " Mhz");
					}else if(value>=1000){
						log.print_inline(value/1000.0, " Khz");
					}else{
						log.print_inline(value, " Hz");
					}

					if(abs(active-value)>active/100){
						log.print_inline(" (currently ");

						if(active>=1000000000){
							log.print_inline(active/1000000.0, " Ghz");
						}else if(active>=1000000){
							log.print_inline(active/1000000.0, " Mhz");
						}else if(active>=1000){
							log.print_inline(active/1000.0, " Khz");
						}else{
							log.print_inline(active, " Hz");
						}
						
						log.print_inline(")");
					}

					if(changeable){
						log.print_inline(" (adjustable)");
					}

				log.print_end();
			}

			if(temps&&voltages){
				log.print_info();
			}

			for(U32 i=0;i<voltages;i++){
				auto name = processor.get_voltage_name(i);
				auto min = processor.get_voltage_min(i);
				auto max = processor.get_voltage_max(i);
				auto value = processor.get_voltage_value(i);

				if(!value) continue;

				log.print_info_start();
				log.print_inline(indent, beforeName, "voltage", i, afterName, ": ", name, " = ", value, " V");
				if(max&&max!=min){
					log.print_inline(" (", min, " - ", max ," V)");
				}
				log.print_end();
				if(max&&max>min){
					print_percent_graph(indent, "  ", value, min, max+10, max);
				}
			}

			if(voltages&&clocks){
				log.print_info();
			}

			for(U32 i=0;i<temps;i++){
				auto name = processor.get_temperature_name(i);
				auto min = celcius_to_kelvin(20);
				auto max = processor.get_temperature_max(i);
				auto value = processor.get_temperature_value(i);

				if(!value) continue;

				log.print_info_start();
				log.print_inline(indent, beforeName, "temp", i, afterName, ": ", name, " = ", kelvin_to_celcius(value), " C");
				if(max){
					log.print_inline(" (", kelvin_to_celcius(max) ," C max)");
				}
				log.print_end();
				if(max&&max>min){
					print_percent_graph(indent, "  ", value, min, max+10, max);
				}
			}

			return clocks>0||temps>0;

		}else if(driver.is_type(driver::Graphics::driverType)){
			auto &graphics = *(driver::Graphics*)&driver;

			U32 framebuffers = graphics.get_framebuffer_count();

			for(auto i=0u;i<framebuffers;i++){
				auto name = graphics.get_framebuffer_name(i);

				log.print_info_start();
				log.print_inline(indent, beforeName, "fb", i, afterName, ": ", name);
				log.print_end();
				log.print_info();

				log.print_info(indent, "Reported video modes:");

				U32 count = 0;
				for(U32 i2=0, modeCount=graphics.get_mode_count(i);i2<modeCount;i2++){
					auto mode = graphics.get_mode(i, i2);

					if(!mode.width) continue;

					log.print_info(indent, "  ", mode.width, "x", mode.height, " @ ", mode.format);
					count++;
				}

				if(!count){
					log.print_warning(indent, "  ", "None found");
				}
			}
		}


		// if(driver.is_type(driver::Serial::driverType)){
		// 	auto &serial = *(driver::Serial*)&driver;
		// }
		// if(driver.is_type(driver::Graphics::driverType)){
		// 	auto &graphics = *(driver::Graphics*)&driver;
		// }
		// if(driver.is_type(driver::Interrupt::driverType)){
		// 	auto &interrupt = *(driver::Interrupt*)&driver;
		// }

		return false;
	}

	void print_summary() {
		auto first = true;
		for(auto &driver:iterate<Driver>()){
			if(!first) log.print_info();
			
			print_driver_summary("", driver);

			first = false;
		}
	}

	auto find_and_activate(DriverType &type, Driver *onBehalf) -> Driver* {
		// try to find an active first
		for(auto &driver:Iterate<Driver>(type)){
			if(!driver.api.is_active()) return &driver;
		}

		// failing that, try to activate a candidate
		for(auto &driver:Iterate<Driver>(type)){
			if(driver.api.is_enabled()){
				if(onBehalf){
					auto section = log.section("REQUEST ", onBehalf->name, " -> ", driver.name);
				}

				if(activate_driver(driver)) return &driver;
			}
		}

		return nullptr;
	}
}
