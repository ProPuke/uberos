#include "Ps2.hpp"

#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/drivers.hpp>
#include <kernel/drivers/x86/system/Acpi.hpp>

namespace driver::system {
	namespace {
		const U16 ioData = 0x60;
		const U16 ioStatus = 0x64;
		const U16 ioCommand = 0x64;

		bool hasPort[(U32)Ps2::Port::max+1] = {};
		Hardware *portDevice[(U32)Ps2::Port::max+1] = {};

		void flush_input() {
			while((U8)Ps2::instance.read_status()&(U8)Ps2::Status::input_full){
				//TODO: pause for a ms or so instead?
				asm("pause");
			};
		}

		auto has_output() -> bool {
			return (U8)Ps2::instance.read_status()&(U8)Ps2::Status::output_full;
		}

		void wait_for_output() {
			//TODO: put in a max timeout before we fail
			while((!(U8)Ps2::instance.read_status()&(U8)Ps2::Status::output_full)){
				//TODO: pause for a ms or so instead?
				asm("pause");
				// arch::x86::ioPort::read8(ioData);
			}
		}

		auto read8(U16 io) -> U8 {
			wait_for_output();
			return arch::x86::ioPort::read8(io);
		}

		void write8(U16 io, U8 data) {
			flush_input();
			arch::x86::ioPort::write8(io, data);
		}

		auto read_expected(U16 io, U8 response) -> Try<> {
			if(read8(io)!=response) return {"Communication failed"};

			return {};
		}

		auto read_ack() -> Try<> {
			return read_expected(ioData, (U8)Ps2::Response::ack);
		}
	}

	auto Ps2::_on_start() -> Try<> {
		auto acpi = drivers::find_and_activate<system::Acpi>();

		if(acpi&&acpi->has_ps2()==Maybe::no) return {"PS/2 ports not available"};

		if(!api.subscribe_ioPort(ioData)||!api.subscribe_ioPort(ioStatus)||!api.subscribe_ioPort(ioCommand)) return {"I/O ports not available"};

		hasPort[(U32)Port::port1] = false;
		hasPort[(U32)Port::port2] = false;
		portDevice[(U32)Port::port1] = nullptr;
		portDevice[(U32)Port::port2] = nullptr;

		wait_for_output();

		write_command(Command::disable_first);
		TRY(read_ack());
		write_command(Command::disable_second);
		TRY(read_ack());

		write_command(Command::read_config);
		auto config = read_data();

		write_command(Command::write_config);
		write_data(config&~((U8)Config::first_irq_mask|(U8)Config::second_irq_mask|(U8)Config::translation));

		log.print_info("config set ", format::Hex8{(U8)(config&~((U8)Config::first_irq_mask|(U8)Config::second_irq_mask|(U8)Config::translation))});

		write_command(Command::test_controller);
		if(read_data()!=0x55) return {"Self-test failed"};

		auto isDual = config&(U8)Config::disable_second_clock;

		write_command(Command::test_first);
		hasPort[(U32)Port::port1] = !read_data();

		if(hasPort[(U32)Port::port1]){
			log.print_info("port 1 (keyboard) detected");

			// config |= (U8)Config::first_irq_mask;
			config &= ~(U8)Config::disable_first_clock;
		}

		if(isDual){
			write_command(Command::test_second);
			hasPort[(U32)Port::port2] = !read_data();

			write_command(Command::enable_second);

			write_command(Command::read_config);
			config = read_data();

			if(!(config&(U8)Config::disable_second_clock)){
				log.print_info("port 2 (mouse) detected");

				// config |= (U8)Config::second_irq_mask;
				config &= ~(U8)Config::disable_second_clock;

				write_command(Command::disable_second);
			}
		}

		if(hasPort[(U32)Port::port1]){
			write_command(Command::enable_first);
		}

		if(hasPort[(U32)Port::port2]){
			write_command(Command::enable_second);
		}

		if(hasPort[(U32)Port::port1]||hasPort[(U32)Port::port2]){
			config &= ~(U8)Config::disable_first_clock;
			config &= ~(U8)Config::disable_second_clock;

			write_command(Command::write_config);
			write_data(config);

			log.print_info("config set ", format::Hex8{(U8)(config)});
		}

		{
			write_command(Ps2::Command::read_config);
			auto config = read_data();

			log.print_info("config is now ", format::Hex8{(U8)(config)});
		}

		return {};
	}

	auto Ps2::_on_stop() -> Try<> {
		return {};
	}

	auto Ps2::has_port(Port port) -> bool {
		return hasPort[(U32)port];
	}

	auto Ps2::get_port_device(Port port) -> Hardware* {
		return portDevice[(U32)port];
	}

	auto Ps2::_install_port_device(Port port, Hardware &hardware) -> bool {
		if(!hasPort[(U32)port]||portDevice[(U32)port]) return false;

		portDevice[(U32)port] = &hardware;
		return true;
	}

	auto Ps2::_uninstall_port_device(Port port, Hardware &hardware) -> bool {
		if(portDevice[(U32)port]!=&hardware) return false;

		portDevice[(U32)port] = nullptr;
		return true;
	}

	auto Ps2::irq1() -> U8 {
		return 1;
	}

	auto Ps2::irq2() -> U8 {
		return 12;
	}

	void Ps2::write_command(Command command) {
		write8(ioCommand, (U8)command);
	}

	void Ps2::write_data(U8 data) {
		write8(ioData, data);
	}

	auto Ps2::has_data() -> bool {
		return has_output();
	}

	auto Ps2::read_data() -> U8 {
		return read8(ioData);
	}

	auto Ps2::read_status() -> Status {
		return (Status)arch::x86::ioPort::read8(ioStatus);
	}

	void Ps2::send_port_command(Port port, U8 command) {
		switch(port){
			case Port::port1:
			break;
			case Port::port2:
				write_command(Command::write_second);
			break;
		}
		write_data((U8)command);
	}

	void Ps2::send_port_data(Port port, U8 data) {
		switch(port){
			case Port::port1:
			break;
			case Port::port2:
				write_command(Command::write_second);
			break;
		}
		write_data(data);
	}
}
