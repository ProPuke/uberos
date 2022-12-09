#include "Raspi.hpp"

#include <common/maths.hpp>

#include <kernel/arch/raspi/mailbox.hpp>

using namespace maths;

namespace mailbox {
	using namespace arch::raspi::mailbox;
}

namespace driver {
	namespace processor {
		namespace {
			const U32 temperatureCount = (U32)mailbox::PropertyMessage::Data::Temperature::max-(U32)mailbox::PropertyMessage::Data::Temperature::min+1;
			const char *temperatureNames[temperatureCount] = {
				"SOC",
			};
		}

		auto Raspi::get_temperature_count() -> U32 { return temperatureCount; }
		auto Raspi::get_temperature_name(U32 index) -> const char* {
			return index<temperatureCount?temperatureNames[index]:"";
		}
		auto Raspi::get_temperature_value(U32 index) -> F32 {
			if(index>0) return 0;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::get_temperature;
			messages[0].data.getTemperature = mailbox::PropertyMessage::Data::Temperature::soc;
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error requesting temperature ", index, " from mailbox");
				return 0;
			}

			auto value = messages[0].data.getTemperatureResult.value;
			return value?celcius_to_kelvin((F32)value/1000.0):0.0;
		}

		auto Raspi::get_temperature_max(U32 index) -> F32 {
			if(index>0) return 0;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::get_max_temperature;
			messages[0].data.getTemperature = mailbox::PropertyMessage::Data::Temperature::soc;
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error requesting max temperature ", index, " from mailbox");
				return 0;
			}

			auto value = messages[0].data.getTemperatureResult.value;
			return value?celcius_to_kelvin((F32)value/1000.0):0.0;
		}

		namespace {
			const U32 voltageCount = (U32)mailbox::PropertyMessage::Data::Voltage::max-(U32)mailbox::PropertyMessage::Data::Voltage::min+1;
			const char *voltageNames[voltageCount] = {
				"Core",
				"SDRAM C",
				"SDRAM P",
				"SDRAM I"
			};
		}

		auto Raspi::get_voltage_count() -> U32 { return voltageCount; }
		auto Raspi::get_voltage_name(U32 index) -> const char* {
			return index<voltageCount?voltageNames[index]:"";
		}

		auto Raspi::get_voltage_value(U32 index) -> F32 {
			if(index>=voltageCount) return 0;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::get_voltage;
			messages[0].data.getVoltage = (mailbox::PropertyMessage::Data::Voltage)((U32)mailbox::PropertyMessage::Data::Voltage::min+index);
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error requesting voltage ", index, " from mailbox");
				return 0;
			}

			return (F32)messages[0].data.getVoltageResult.value/1000000.0;
		}

		auto Raspi::get_voltage_min(U32 index) -> F32 {
			if(index>=voltageCount) return 0;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::get_min_voltage;
			messages[0].data.getVoltage = (mailbox::PropertyMessage::Data::Voltage)((U32)mailbox::PropertyMessage::Data::Voltage::min+index);
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error requesting min voltage ", index, " from mailbox");
				return 0;
			}

			return (F32)messages[0].data.getVoltageResult.value/1000000.0;
		}

		auto Raspi::get_voltage_max(U32 index) -> F32 {
			if(index>=voltageCount) return 0;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::get_max_voltage;
			messages[0].data.getVoltage = (mailbox::PropertyMessage::Data::Voltage)((U32)mailbox::PropertyMessage::Data::Voltage::min+index);
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error requesting max voltage ", index, " from mailbox");
				return 0;
			}

			return (F32)messages[0].data.getVoltageResult.value/1000000.0;
		}

		auto Raspi::can_set_voltage(U32 index) -> bool { return index<voltageCount; }
		auto Raspi::set_voltage_value(U32 index, F32 set) -> bool {
			if(index>=voltageCount) return false;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::set_voltage;
			messages[0].data.setVoltage.voltage = (mailbox::PropertyMessage::Data::Voltage)((U32)mailbox::PropertyMessage::Data::Voltage::min+index);
			messages[0].data.setVoltage.value = set*1000000; //TODO: this might need special treatment in lower values (see mailbox notes)
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error setting voltage ", index, " via mailbox");
				return false;
			}

			return true;
		}


		namespace {
			const U32 clockCount = (U32)mailbox::PropertyMessage::Data::Clock::max-(U32)mailbox::PropertyMessage::Data::Clock::min+1;
			struct Clock {const char *name; U32 id; };
			Clock clocks[clockCount] = { //NOTE:all ids start at 0 and are sequential, they're just reordered for prettier defaults
				{ "CORE", 3 },
				{ "ARM", 2 },
				{ "V3D", 4 },
				{ "EMMC", 0 },
				{ "UART", 1 },
				{ "H264", 5 },
				{ "ISP", 6 },
				{ "SDRAM", 7},
				{ "Pixel", 8 },
				{ "PWM", 9 },
				{ "HEVC", 10 },
				{ "EMMC2", 11 },
				{ "M2MC", 12 },
				{ "Pixel BVB", 13 },
			};
		}

		auto Raspi::get_clock_count() -> U32 { return clockCount; }
		auto Raspi::get_clock_name(U32 index) -> const char* {
			return index<clockCount?clocks[index].name:"";
		}

		auto Raspi::get_clock_value(U32 index) -> U32 {
			if(index>=clockCount) return 0;

			auto id = clocks[index].id;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::get_clock;
			messages[0].data.getClock = (mailbox::PropertyMessage::Data::Clock)((U32)mailbox::PropertyMessage::Data::Clock::min+id);
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error requesting clock ", id, " from mailbox");
				return 0;
			}

			return messages[0].data.getClockResult.rate;
		}

		auto Raspi::get_clock_active_value(U32 index) -> U32 {
			if(index>=clockCount) return 0;

			auto id = clocks[index].id;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::get_actual_clock;
			messages[0].data.getClock = (mailbox::PropertyMessage::Data::Clock)((U32)mailbox::PropertyMessage::Data::Clock::min+id);
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error requesting clock ", id, " from mailbox");
				return 0;
			}

			return messages[0].data.getClockResult.rate;
		}

		auto Raspi::get_clock_min(U32 index) -> U32 {
			if(index>=clockCount) return 0;

			auto id = clocks[index].id;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::get_min_clock;
			messages[0].data.getClock = (mailbox::PropertyMessage::Data::Clock)((U32)mailbox::PropertyMessage::Data::Clock::min+id);
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error requesting min voltage ", id, " from mailbox");
				return 0;
			}

			return messages[0].data.getClockResult.rate;
		}

		auto Raspi::get_clock_max(U32 index) -> U32 {
			if(index>=clockCount) return 0;

			auto id = clocks[index].id;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::get_max_clock;
			messages[0].data.getClock = (mailbox::PropertyMessage::Data::Clock)((U32)mailbox::PropertyMessage::Data::Clock::min+id);
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error requesting max voltage ", id, " from mailbox");
				return 0;
			}

			return messages[0].data.getClockResult.rate;
		}

		auto Raspi::can_set_clock(U32 index) -> bool { return index<clockCount; }
		auto Raspi::set_clock_value(U32 index, U32 set) -> bool {
			if(index>=clockCount) return false;

			auto id = clocks[index].id;

			mailbox::PropertyMessage messages[2];
			messages[0].tag = mailbox::PropertyTag::set_clock;
			messages[0].data.setClock.clock = (mailbox::PropertyMessage::Data::Clock)((U32)mailbox::PropertyMessage::Data::Clock::min+id);
			messages[0].data.setClock.rate = set;
			messages[0].data.setClock.skipSettingTurbo = 0;
			messages[1].tag = mailbox::PropertyTag::null_tag;

			if(!mailbox::send_messages(messages)){
				log::print_error("Error setting clock ", id, " via mailbox");
				return false;
			}

			return true;
		}

	}
}
