#pragma once

#include <drivers/Hardware.hpp>

#include <common/Try.hpp>

namespace driver::system {
	//TODO: merge into a 8259 PIC driver?

	struct Ps2 final: Hardware {
		DRIVER_INSTANCE(Ps2, 0x68d9019a, "ps2", "PS/2", Hardware)

		enum struct Port: U8 {
			port1,
			port2,
			max = port2
		};

		enum struct Command: U8 {
			read_config = 0x20,
			write_config = 0x60,
			disable_second = 0xa7,
			enable_second = 0xa8,
			test_second = 0xa9,
			test_first = 0xab,
			disable_first = 0xad,
			enable_first = 0xae,
			write_second = 0xd4,
			test_controller = 0xaa,
		};

		enum struct Response {
			ack = 0xfa
		};

		enum struct Config: U8 {
			first_irq_mask = 1<<0,
			second_irq_mask = 1<<1,
			disable_first_clock = 1<<4,
			disable_second_clock = 1<<5,
			translation = 1<<6,
		};

		enum struct Status: U8 {
			output_full = 1<<0,
			input_full  = 1<<1,
			mouse_byte  = 1<<5, // unreliable?
		};

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto has_port(Port) -> bool;
		auto get_port_device(Port) -> Hardware*;
		auto _install_port_device(Port, Hardware&) -> bool;
		auto _uninstall_port_device(Port, Hardware&) -> bool;

		auto irq1() -> U8;
		auto irq2() -> U8;

		void write_command(Command);
		void write_data(U8);
		auto has_data() -> bool;
		auto read_data() -> U8;
		auto read_status() -> Status;

		void send_port_command(Port port, U8 command);
		void send_port_data(Port port, U8 data);
	};
}
