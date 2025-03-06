#include "CmosRtc.hpp"

#include <drivers/x86/system/Acpi.hpp>

#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/arch/x86/nmi.hpp>
#include <kernel/drivers.hpp>

namespace driver::clock {
	namespace {
		const U16 ioAddress = 0x70;
		const U16 ioData = 0x71;

		auto read_register(U8 reg) -> U8 {
			arch::x86::ioPort::write8(ioAddress, reg|(arch::x86::nmi::isEnabled()?0x00:0x80));
			return arch::x86::ioPort::read8(ioData);
		}

		void write_register(U8 reg, U8 value) {
			arch::x86::ioPort::write8(ioAddress, reg|(arch::x86::nmi::isEnabled()?0x00:0x80));
			arch::x86::ioPort::write8(ioData, value);
		}

		auto parse_bcd(U8 x){
			return (x&0xf)+(x>>4)*10;
		}
	}

	auto CmosRtc::_on_start() -> Try<> {
		auto acpi = drivers::find_and_activate<system::Acpi>(this);

		if(acpi&&acpi->has_cmos_rtc()==Maybe::no) return {"CMOS RTC not available"};

		return {};
	}
	auto CmosRtc::_on_stop() -> Try<> {
		return {};
	}

	auto CmosRtc::get_date() -> Date {
		auto statusRegisterB = read_register(0x0b);
		auto isBcd = !(statusRegisterB&(1<<2));

		auto century = read_register(0x32); // MAYBE
		U16 year = read_register(0x09);
		auto month = read_register(0x08); // 1..12
		auto date = read_register(0x07); // 1..31

		if(isBcd){
			century = parse_bcd(century);
			year = parse_bcd(year);
			month = parse_bcd(month);
			date = parse_bcd(date);
		}

		if(century>=19&&century<=21){ // probably valid?
			year += century*100;

		}else{ // otherwise 2000s ¯\_(ツ)_/¯
			year += 2000;
		}

		month -= 1; // to 0..11
		date -= 1; // to 0..30

		return { year, month, date };
	}

	auto CmosRtc::get_time() -> Time {
		auto statusRegisterB = read_register(0x0b);
		auto is12Hr = !(statusRegisterB&(1<<1));
		auto isBcd = !(statusRegisterB&(1<<2));

		auto hours = read_register(0x04);
		auto minutes = read_register(0x02);
		auto seconds = read_register(0x00);

		if(isBcd){
			if(is12Hr){
				bool isPm = hours&0x80;
				hours = hours&0x7f;
				hours = parse_bcd(hours);
				if(hours==12) hours = 0;
				if(isPm) hours += 12;
			}else{
				hours = parse_bcd(hours);
			}

			minutes = parse_bcd(minutes);
			seconds = parse_bcd(seconds);

		}else{
			if(is12Hr){
				bool isPm = hours&0x80;
				hours = hours&0x7f;
				if(hours==12) hours = 0;
				if(isPm) hours += 12;
			}
		}

		return { hours, minutes, seconds};
	}
}
