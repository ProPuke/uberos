#include "drivers.hpp"

#include <drivers/Graphics.hpp>
#include <drivers/Interrupt.hpp>
#include <drivers/Processor.hpp>
#include <drivers/Serial.hpp>

#include <kernel/Driver.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/Log.hpp>

#include <common/PodArray.hpp>
#include <common/format.hpp>
#include <common/maths.hpp>

static Log log("drivers");

using namespace maths;

namespace drivers {
	constinit EventEmitter<Event> events;

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

	void install_driver(Driver &driver) {
		drivers.push_back(driver);

		// auto priority = driver.get_priority();

		// Driver *next = nullptr;

		// for(auto existing=drivers.head;existing;existing=existing->next){
		// 	if(existing->get_priority()<priority){
		// 		next = existing;
		// 		break;
		// 	}
		// }

		// if(next){
		// 	drivers.insert_before(*next, driver);
		// }else{
		// 	drivers.push_back(driver);
		// }

		events.trigger({
			type: Event::Type::driverInstalled,
			driverInstalled: { &driver }
		});
	}

	auto start_driver(Driver &driver) -> Try<> {
		if(driver.api.is_active()) return {};

		auto section = log.section("START ", driver.type->name, "...");

		Driver *disabledDevice = nullptr;

		if(auto result = driver.api.start_driver(); !result) {
			log.print_warning("Unable to start ", driver.type->name, ": ", result.errorMessage);
			if(!driver.api.is_active()){
				return result;
			}
		};

		if(!driver.api.is_active()){
			log.print_error("Failed starting ", driver.type->name);
			//if we failed to start the driver, and stopped a previous in order to try, then restore it
			if(disabledDevice){
				TRY(start_driver(*disabledDevice));
			}
			return {"Unable to start driver"};
		}

		events.trigger({
			type: Event::Type::driverStarted,
			driverStarted: { &driver }
		});

		return {};
	}

	auto stop_driver(Driver &driver) -> Try<> {
		if(!driver.api.is_active()) return {};
		if(!driver.can_stop_driver()) return {"This driver cannot be stopped"};

		auto section = log.section("STOP ", driver.type->name);

		if(auto result = driver.api.stop_driver(); !result) {
			log.print_error("Error stopping ", driver.type->name, ": ", result.errorMessage);
			return result;
		}

		if(!driver.api.is_disabled()){
			//TODO: driver might have enter a failed state while stopping. Should report this in some other way, since the driver IS now technically stopped?
			log.print_error("Failed stopping ", driver.type->name);
			return {"Unable to stop driver"};
		}

		events.trigger({
			type: Event::Type::driverStopped,
			driverStopped: { &driver, nullptr }
		});

		return {};
	}

	auto restart_driver(Driver &driver) -> Try<> {
		if(!driver.api.is_active()){
			return start_driver(driver);
		}

		if(!driver.can_restart_driver()) return {"This driver cannot be restarted"};

		auto section = log.section("RESTART ", driver.type->name);

		if(auto result = driver.api.restart_driver(); !result) {
			log.print_error("Error restarting ", driver.type->name, ": ", result.errorMessage);

			events.trigger({
				type: Event::Type::driverStopped,
				driverStopped: { &driver, result.errorMessage }
			});

			if(!driver.api.is_active()){
				return result;
			}
		}

		if(!driver.api.is_active()){
			log.print_error("Failed restarting ", driver.type->name);
			return {"Unable to restart driver"};
		}

		events.trigger({
			type: Event::Type::driverStarted,
			driverStarted: { &driver }
		});

		return {};
	}

	auto find_first(DriverTypeId typeId) -> Driver* {
		for(Driver *driver = drivers.head; driver; driver = driver->next){
			if(driver->is_type(typeId)) return driver;
		}

		return nullptr;
	}
	auto find_next(Driver &after, DriverTypeId typeId) -> Driver* {
		for(Driver *driver = after.next; driver; driver = driver->next){
			if(driver->is_type(typeId)) return driver;
		}

		return nullptr;
	}

	auto is_memory_in_use(Physical<void> address, size_t size) -> bool {
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

	namespace {
		void on_driver_irq(U8 irq, void *data){
			auto &driver = *(Driver*)data;
			driver._on_irq(irq);
		};
	}

	auto _subscribe_driver_to_irq(Driver &driver, U8 irq) -> Try<> {
		TRY(exceptions::irq::subscribe(irq, on_driver_irq, &driver));
		return {};
	}

	void _unsubscribe_driver_from_irq(Driver &driver, U8 irq) {
		exceptions::irq::unsubscribe(irq, on_driver_irq, &driver);
	}

	void print_driver_summary(const char *indent, Driver &driver) {
		log.print_info_start();
		log.print_inline(indent);

		if(driver.api.is_active()){
			log.print_inline("[+] ");
		}else if(driver.api.is_failed()){
			log.print_inline("[!] ");
		}else if(driver.api.is_disabled()){
			log.print_inline("[x] ");
		}else if(driver.api.is_enabled()){
			log.print_inline("[.] ");
		}else{
			log.print_inline("[?] ");
		}

		log.print_inline(driver.type->name, " (", driver.type->description, ')');
		for(auto parent = driver.type->parentType; parent&&parent->parentType; parent = parent->parentType) {
			logging::print_inline(" / ", parent->description);
		}

		// if(driver.address){
		// 	log.print_inline(" @ ", to_string(driver.address));
		// }
		log.print_end();

		// log.print_info("   State: ", driver.state);
		for(auto &subscription:driver.api.subscribedMemory){
			#ifdef _64BIT
				log.print_info(indent, "   Memory: ", format::Hex64{subscription.start.address}, " - ", format::Hex64{subscription.end.address});
			#else
				log.print_info(indent, "   Memory: ", format::Hex32{subscription.start.address}, " - ", format::Hex32{subscription.end.address});
			#endif
		}
		if(driver.api.subscribedIoPorts.length>0){
			log.print_info_start();
			log.print_inline(indent, "   I/O Ports:");
			for(auto port:driver.api.subscribedIoPorts) {
				log.print_inline(' ', port);
			}
			log.print_end();
		}
		if(driver.api.subscribedIrqs.has_any()){
			log.print_info_start();
			log.print_inline(indent, "   IRQs:");
			for(auto i=0u;i<256;i++){
				if(driver.api.subscribedIrqs.get(i)){
					log.print_inline(' ', i);
				}
			}
			log.print_end();
		}
		if(driver.api.subscribedInterrupts.has_any()){
			log.print_info_start();
			log.print_inline(indent, "   Interrupts:");
			for(auto i=0u;i<256;i++){
				if(driver.api.subscribedInterrupts.get(i)){
					log.print_inline(' ', i);
				}
			}
			log.print_end();
		}
		if(driver.api.subscribedPciDevices.length > 0){
			log.print_info_start();
			log.print_inline(indent, "   PCI Devices:");
			for(auto pciDevice:driver.api.subscribedPciDevices) {
				log.print_inline(' ', format::Hex8{pciDevice->bus, false}, ':', format::Hex8{pciDevice->device, false}, '.', pciDevice->function);
			}
			log.print_end();
		}
		if(auto processor = driver.as_type<driver::Processor>()){
			log.print_info(indent, "   Architecture: ", processor->processor_arch);
			log.print_info(indent, "   Cores: ", processor->processor_cores);

			auto speed = processor->get_clock_value(0);
			auto temp = processor->get_temperature_value(0);

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
				auto maxTemp = processor->get_temperature_max(0);

				if(maxTemp&&maxTemp>minTemp){
					print_percent_graph(indent, "   ", temp, minTemp, maxTemp+10, maxTemp);
				}
			}

			// for(U32 i=0;i<processor->get_voltage_count();i++){
			// 	log.print_info(indent, "   ", processor->get_voltage_name(i), " = ", processor->get_voltage_value(i), " V");
			// }

			// for(U32 i=0;i<processor->get_clock_count();i++){
			// 	log.print_info(indent, "   ", processor->get_clock_name(i), " = ", processor->get_clock_value(i)/1000000.0f, " Mhz");
			// }

			// for(U32 i=0;i<processor->get_temperature_count();i++){
			// 	log.print_info(indent, "   ", processor->get_temperature_name(i), " = ", processor->get_temperature_value(i), " K");
			// }
		}
		if(auto serial = driver.as_type<driver::Serial>()){
			if(serial->api.is_enabled()){
				log.print_info(indent, "   Baud: ", serial->get_active_baud());
			}
		}
		if(auto graphics = driver.as_type<driver::Graphics>()){
			const auto framebufferCount = graphics->get_framebuffer_count();
			log.print_info(indent, "   Framebuffers: ", framebufferCount);
			for(auto i=0u;i<framebufferCount;i++){
				auto defaultMode = graphics->get_default_mode(i);

				if(defaultMode.width){
					log.print_info(indent, "    ", graphics->get_framebuffer_name(i), " - Default mode: ", defaultMode.width, "x", defaultMode.height, " @ ", defaultMode.format);
				}else{
					log.print_info(indent, "    ", graphics->get_framebuffer_name(i), " - Default mode: None");
				}
			}
		}
		if(auto irq = driver.as_type<driver::Interrupt>()){
			log.print_info(indent, "   Provided IRQs: ", irq->min_irq, " - ", irq->max_irq);
		}
	}

	bool print_driver_details(const char *indent, Driver &driver, const char *beforeName, const char *afterName) {
		if(auto processor = driver.as_type<driver::Processor>()){
			auto clocks = processor->get_clock_count();
			auto voltages = processor->get_voltage_count();
			auto temps = processor->get_temperature_count();

			for(U32 i=0;i<clocks;i++){
				auto name = processor->get_clock_name(i);
				// auto min = processor->get_clock_min(i);
				// auto max = processor->get_clock_max(i);
				auto value = processor->get_clock_value(i);
				auto changeable = processor->can_set_clock(i);
				auto active = processor->get_clock_active_value(i);

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
				auto name = processor->get_voltage_name(i);
				auto min = processor->get_voltage_min(i);
				auto max = processor->get_voltage_max(i);
				auto value = processor->get_voltage_value(i);

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
				auto name = processor->get_temperature_name(i);
				auto min = celcius_to_kelvin(20);
				auto max = processor->get_temperature_max(i);
				auto value = processor->get_temperature_value(i);

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

		}else if(auto graphics = driver.as_type<driver::Graphics>()){
			U32 framebuffers = graphics->get_framebuffer_count();

			for(auto i=0u;i<framebuffers;i++){
				auto name = graphics->get_framebuffer_name(i);

				log.print_info_start();
				log.print_inline(indent, beforeName, "fb", i, afterName, ": ", name);
				log.print_end();
				log.print_info();

				log.print_info(indent, "Reported video modes:");

				U32 count = 0;
				for(U32 i2=0, modeCount=graphics->get_mode_count(i);i2<modeCount;i2++){
					auto mode = graphics->get_mode(i, i2);

					if(!mode.width) continue;

					log.print_info(indent, "  ", mode.width, "x", mode.height, " @ ", mode.format);
					count++;
				}

				if(!count){
					log.print_warning(indent, "  ", "None found");
				}
			}
		}


		// if(auto serial = driver.as_type<driver::Serial>()){
		// }
		// if(auto graphics = driver.as_type<driver::Graphics>()){
		// }
		// if(auto interrupt = driver.as_type<driver::Interrupt>()){
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

	auto find_and_activate(DriverTypeId typeId, Driver *onBehalf) -> Driver* {
		// try to find an active first
		if(auto active = find_active(typeId, onBehalf)){
			return active;
		}

		// failing that, try to activate a candidate
		for(auto &driver:Iterate<Driver>(typeId)){
			if(driver.api.is_enabled()){
				if(onBehalf){
					log.print_info("REQUEST ", onBehalf->type->name, " -> ", driver.type->name);
				}

				if(start_driver(driver)) return &driver;
			}
		}

		return nullptr;
	}

	auto find_active(DriverTypeId typeId, Driver *onBehalf) -> Driver* {
		for(auto &driver:Iterate<Driver>(typeId)){
			if(driver.api.is_active()) return &driver;
		}

		return nullptr;
	}
}
