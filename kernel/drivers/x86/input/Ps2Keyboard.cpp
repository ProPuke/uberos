#include "Ps2Keyboard.hpp"

#include <kernel/drivers/x86/system/Ps2.hpp>

namespace driver::input {
	namespace {
		typedef system::Ps2 Ps2;

		DriverReference<system::Ps2> ps2{nullptr, [](void*){
			Ps2Keyboard::instance.log.print_warning("PS/2 drivers halted - terminating");
			if(!drivers::stop_driver(Ps2Keyboard::instance)){
				Ps2Keyboard::instance.api.fail_driver("PS/2 drivers halted");
			}
		}, nullptr};
		U8 irq;

		auto read_expected(U8 response) -> Try<> {
			if(ps2->read_data()!=response) return {"Communication failed"};

			return {};
		}

		auto read_ack() -> Try<> {
			return read_expected((U8)Ps2::Response::ack);
		}

		void wait_for_output() {
			//TODO: put in a max timeout before we fail
			while((!(U8)Ps2::instance.read_status()&(U8)Ps2::Status::output_full)){
				//TODO: pause for a ms or so instead?
				asm("pause");
				// arch::x86::ioPort::read8(ioData);
			}
		}

		auto get_scancode() -> Try<U8> {
			Ps2Keyboard::instance.send_ps2_command(Ps2Keyboard::Ps2Command::scancode);
			TRY(read_ack());

			Ps2Keyboard::instance.send_ps2_data((U8)Ps2Keyboard::Ps2CommandScancode::get);
			TRY(read_ack());

			return ps2->read_data()&0b11;
		}

		//TODO: rates
		auto set_repeat() -> Try<> {
			Ps2Keyboard::instance.send_ps2_command(Ps2Keyboard::Ps2Command::setRepeat);
			TRY(read_ack());

			Ps2Keyboard::instance.send_ps2_data(0);
			TRY(read_ack());

			return {};
		}
	}

	auto Ps2Keyboard::_on_start() -> Try<> {
		ps2 = drivers::find_and_activate<system::Ps2>();
		if(!ps2) return {"PS/2 not available"};

		if(!ps2->has_port(Ps2::Port::port1)) return {"PS/2 port 1 not present"};
		if(ps2->get_port_device(Ps2::Port::port1)) return {"PS/2 port 1 already in use"};

		auto port2Device = ps2->get_port_device(Ps2::Port::port2);
		defer {
			if(port2Device){
				ps2->send_port_command(Ps2::Port::port2, (U8)Ps2Command::enableReporting);
				ps2->read_data();
			}
		};

		if(port2Device){
			ps2->send_port_command(Ps2::Port::port2, (U8)Ps2Command::disableReporting);
			TRY(read_ack());
		}

		irq = ps2->irq1();

		U8 scancode = TRY_RESULT(get_scancode());
		log.print_info("scancode: ", scancode);

		TRY(set_repeat());

		// send_ps2_command(Ps2Command::enableReporting);
		// TRY(read_ack());

		if(!ps2->_install_port_device(Ps2::Port::port1, *this)) return {"Unable to install PS/2 port 1 device"};
		
		api.subscribe_irq(irq);

		{ // enable irqs
			ps2->write_command(Ps2::Command::read_config);
			auto config = ps2->read_data();

			ps2->write_command(Ps2::Command::write_config);
			ps2->write_data(config|(U8)Ps2::Config::first_irq_mask);
		}

		return {};
	}

	auto Ps2Keyboard::_on_stop() -> Try<> {
		if(ps2){
			ps2->_uninstall_port_device(Ps2::Port::port1, *this);
		}

		return {};
	}

	void Ps2Keyboard::_on_irq(U8 _irq) {
		if(_irq!=irq) return;

		while(Ps2::instance.has_data()){
			auto status = ps2->read_status();
			if((U8)status&(U8)Ps2::Status::mouse_byte) break;

			U8 byte = ps2->read_data();

			log.print_info("scancode ", byte);
		}
	}

	void Ps2Keyboard::send_ps2_command(Ps2Command command) {
		ps2->send_port_command(Ps2::Port::port1, (U8)command);
	}

	void Ps2Keyboard::send_ps2_data(U8 data) {
		ps2->send_port_data(Ps2::Port::port1, data);
	}
}
