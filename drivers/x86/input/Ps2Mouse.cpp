#include "Ps2Mouse.hpp"

#include <drivers/x86/system/Ps2.hpp>

#include <kernel/DriverReference.hpp>
#include <kernel/drivers.hpp>

namespace driver::input {
	namespace {
		typedef system::Ps2 Ps2;

		DriverReference<Ps2> ps2{nullptr, [](void*){
			Ps2Mouse::instance.log.print_warning("PS/2 drivers halted - terminating");
			if(!drivers::stop_driver(Ps2Mouse::instance)){
				Ps2Mouse::instance.api.fail_driver("Ps/2 drivers halted");
			}
		}, nullptr};
		U8 irq;

		U8 buttonCount = 3;
		bool hasWheel = false;
		I32 positionX = 0;
		I32 positionY = 0;
		I32 scale = 1;

		auto read_expected(U8 response) -> Try<> {
			if(ps2->read_data()!=response) return Failure{"Communication failed"};

			return {};
		}

		auto read_ack() -> Try<> {
			return read_expected((U8)Ps2::Response::ack);
		}

		auto set_rate(U8 rate) -> Try<> {
			Ps2Mouse::instance.send_ps2_command(Ps2Mouse::Ps2Command::sampleRate);
			TRY(read_ack());

			Ps2Mouse::instance.send_ps2_data(rate);
			TRY(read_ack());

			return {};
		}

		auto set_scale(I32 _scale) -> Try<> {
			if(_scale!=1&&_scale!=2) return {};

			Ps2Mouse::instance.send_ps2_command(_scale==2?Ps2Mouse::Ps2Command::setScale2:Ps2Mouse::Ps2Command::setScale1);
			TRY(read_ack());

			scale = _scale;

			return {};
		}

		void wait_for_output() {
			//TODO: put in a max timeout before we fail
			while((!(U8)Ps2::instance.read_status()&(U8)Ps2::Status::output_full)){
				//TODO: pause for a ms or so instead?
				asm("pause");
				// arch::x86::ioPort::read8(ioData);
			}
		}

		union __attribute__((packed)) Packet {
			struct __attribute__((packed)) {
				// 3 button
				bool leftButton:1;
				bool rightButton:1;
				bool middleButton:1;
				U32 _1:1;
				bool xSign:1;
				bool ySign:1;
				bool xOverflow:1;
				bool yOverflow:1;

				U8 xChange;
				U8 yChange;

				union __attribute__((packed)) {
					struct __attribute__((packed)) {
						I8 wheelChange;
					} _3Buttons;

					struct __attribute__((packed)) {
						// wheel
						I8 wheelChange:4;

						// 5 button
						bool button4:1;
						bool button5:1;
						U32 _0:2;
					} _5Buttons;
				} wheelMouse;
			};

			U8 data[4];
		} packet;

		U32 packetBytes = 0;

		void trigger_event(Mouse::Event event) {
			event.instance = &Ps2Mouse::instance;
			Ps2Mouse::instance.events.trigger(event);
			Ps2Mouse::allEvents.trigger(event);
		}
	}

	auto Ps2Mouse::_on_start() -> Try<> {
		ps2 = drivers::find_and_activate<Ps2>(this);
		if(!ps2) return Failure{"PS/2 not available"};

		if(!ps2->has_port(Ps2::Port::port2)) return Failure{"PS/2 port 2 not present"};
		if(ps2->get_port_device(Ps2::Port::port2)) return Failure{"PS/2 port 2 already in use"};
		
		irq = ps2->irq2();

		for(auto i=0u;i<maxButtons;i++){
			buttonState[i] = 0;
		}

		buttonCount = 3;
		hasWheel = false;
		positionX = 0;
		positionY = 0;

		packetBytes = 0;
		packet.data[0] = 0;
		packet.data[1] = 0;
		packet.data[2] = 0;
		packet.data[3] = 0;

		// { // disable irqs
		// 	ps2->write_command(Ps2::Command::read_config);
		// 	auto config = ps2->read_data();

		// 	ps2->write_command(Ps2::Command::write_config);
		// 	ps2->write_data(config&~(U8)Ps2::Config::second_irq_mask);
		// }

		auto port1Device = ps2->get_port_device(Ps2::Port::port1);
		defer {
			if(port1Device){
				ps2->send_port_command(Ps2::Port::port1, (U8)Ps2Command::enableReporting);
				ps2->read_data();
			}
		};

		if(port1Device){
			ps2->send_port_command(Ps2::Port::port1, (U8)Ps2Command::disableReporting);
			TRY(read_ack());
		}

		send_ps2_command(Ps2Command::reset);
		TRY(read_ack());
		TRY(read_expected(0xaa));
		TRY(read_expected(0x00));

		send_ps2_command(Ps2Command::disableReporting);
		TRY(read_ack());

		// secret mousewheel handshake
		TRY(set_rate(200));
		TRY(set_rate(100));
		TRY(set_rate(80));

		{ // identify device
			send_ps2_command(Ps2Command::identify);
			TRY(read_ack());
			auto a = ps2->read_data();
			auto b = 0; //ps2->read_data(); //TODO: support timeout (it might only return 1 byte)
			U16 deviceId = (U16)a<<8 | b;

			switch(deviceId){
				case 0x0000: // standard mouse
					buttonCount = 3;
				break;
				case 0x0300: // wheel mouse
					hasWheel = true;

					// 5 button konami code..
					TRY(set_rate(200));
					TRY(set_rate(200));
					TRY(set_rate(80));

					send_ps2_command(Ps2Command::identify);
					TRY(read_ack());

					a = ps2->read_data();
					b = 0; //ps2->read_data(); //TODO: support timeout (it might only return 1 byte)
					deviceId = (U16)a<<8 | b;

					if(deviceId==0x0400){ // 5 button mouse
						buttonCount = 5;
					}else{
						buttonCount = 3;
					}
				break;
				case 0x0400: // 5 button mouse (don't think is returned without the wheel+konami)
					buttonCount = 5;
				break;
				default:
					log.print_info("unknown device type: ", format::Hex16{deviceId});
					return Failure{"device not identified as a mouse"};
			}
		}

		log.print_info("buttons: ", buttonCount);
		log.print_info("wheel: ", hasWheel?"yes":"no");

		TRY(set_rate(200));

		send_ps2_command(Ps2Command::enableReporting);
		TRY(read_ack());
		// send_ps2_command(Ps2Command::reset);

		// while(true){
		// 	auto data = ps2->read_data();
		// 	log.print_info(format::Hex8{data});
		// }

		if(!ps2->_install_port_device(Ps2::Port::port2, *this)) return Failure{"Unable to install PS/2 port 2 device"};

		TRY(api.subscribe_irq(irq));

		if(true){ // enable irqs
			ps2->write_command(Ps2::Command::read_config);
			auto config = ps2->read_data();

			ps2->write_command(Ps2::Command::write_config);
			ps2->write_data((config|(U8)Ps2::Config::second_irq_mask));
		}

		return {};
	}

	auto Ps2Mouse::_on_stop() -> Try<> {
		if(ps2){
			ps2->_uninstall_port_device(Ps2::Port::port2, *this);
		}

		return {};
	}

	void Ps2Mouse::_on_irq(U8 _irq) {
		if(_irq!=irq) return;

		I32 rescale = 0;

		// consolidate all mousemoves to a single event (to avoid multiple updates occuring during fast mousemoves slowing things down)
		// this does mean that queued moves AND clicks will click at the wrong earlier position
		// TODO: improve this by applying and flushing pendingMotion early when click events are encountered in this loop
		I32 pendingMotion[2] = {0, 0};

		while(Ps2::instance.has_data()){
			auto status = ps2->read_status();
			if(!((U8)status&(U8)Ps2::Status::mouse_byte)) break;

			packet.data[packetBytes++] = ps2->read_data();
			// packet.data[packetBytes++] =  arch::x86::ioPort::read8(0x60);

			if(packetBytes==1&&packet._1!=true){
				// Out of sync, reset packet position
				packetBytes = 0;
				packet.data[0] = 0;
				continue;
			}

			if(packetBytes<(hasWheel||buttonCount>3?4u:3u)) continue;

			if(buttonState[0] != packet.leftButton){
				buttonState[0] = packet.leftButton;
				// log.print_info("button 0 = ", buttonState[0]);
				if(buttonState[0]){
					trigger_event({
						type: Event::Type::pressed,
						pressed: {0}
					});
				}else{
					trigger_event({
						type: Event::Type::released,
						released: {0}
					});
				}
			}
			if(buttonState[1] != packet.rightButton){
				buttonState[1] = packet.rightButton;
				// log.print_info("button 1 = ", buttonState[1]);
				if(buttonState[1]){
					trigger_event({
						type: Event::Type::pressed,
						pressed: {1}
					});
				}else{
					trigger_event({
						type: Event::Type::released,
						released: {1}
					});
				}
			}
			if(buttonState[2] != packet.middleButton){
				buttonState[2] = packet.middleButton;
				// log.print_info("button 2 = ", buttonState[2]);
				if(buttonState[2]){
					trigger_event({
						type: Event::Type::pressed,
						pressed: {2}
					});
				}else{
					trigger_event({
						type: Event::Type::released,
						released: {2}
					});
				}
			}

			if(buttonCount>3){
				if(buttonState[3] != packet.wheelMouse._5Buttons.button4){
					buttonState[3] = packet.wheelMouse._5Buttons.button4;
					// log.print_info("button 3 = ", buttonState[3]);
					if(buttonState[3]){
						trigger_event({
							type: Event::Type::pressed,
							pressed: {3}
						});
					}else{
						trigger_event({
							type: Event::Type::released,
							released: {3}
						});
					}
				}
				if(buttonState[4] != packet.wheelMouse._5Buttons.button5){
					buttonState[4] = packet.wheelMouse._5Buttons.button5;
					// log.print_info("button 4 = ", buttonState[4]);
					if(buttonState[4]){
						trigger_event({
							type: Event::Type::pressed,
							pressed: {4}
						});
					}else{
						trigger_event({
							type: Event::Type::released,
							released: {4}
						});
					}
				}
			}

			auto xMotion = (+((U8)packet.xChange - (packet.xSign?0x100:0))) * scale;
			auto yMotion = (-((U8)packet.yChange - (packet.ySign?0x100:0))) * scale;
			// auto xMotion = (U8)packet.xChange - (packet.data[0]<<4 & 0x100);
			// auto yMotion = -((U8)packet.yChange - (packet.data[0]<<3 & 0x100));

			if(xMotion||yMotion){
				pendingMotion[0] += xMotion;
				pendingMotion[1] += yMotion;

				if(false){ // auto-rescale on high movements
					if(scale==1&&(maths::abs(xMotion)>=127||maths::abs(yMotion)>=127)){
						rescale = 2;

					}else if(scale==2&&(maths::abs(xMotion)<64||maths::abs(yMotion)<64)){
						rescale = 1;
					}
				}
			}

			if(hasWheel){
				if(buttonCount>3){
					if(packet.wheelMouse._5Buttons.wheelChange){
						// log.print_info("wheel = ", packet.wheelMouse._5Buttons.wheelChange);
						trigger_event({
							type: Event::Type::scrolled,
							scrolled: packet.wheelMouse._5Buttons.wheelChange
						});
					}
				}else{
					if(packet.wheelMouse._3Buttons.wheelChange){
						// log.print_info("wheel = ", packet.wheelMouse._3Buttons.wheelChange);
						trigger_event({
							type: Event::Type::scrolled,
							scrolled: packet.wheelMouse._5Buttons.wheelChange
						});
					}
				}
			}

			packetBytes = 0;
			packet.data[0] = 0;
			packet.data[1] = 0;
			packet.data[2] = 0;
			packet.data[3] = 0;
		}

		if(pendingMotion[0]||pendingMotion[1]){
			trigger_event({
				type: Event::Type::moved,
				moved: {
					x: pendingMotion[0],
					y: pendingMotion[1]
				}
			});
		}

		if(rescale==1){
			send_ps2_command(Ps2Command::disableReporting);
			if(read_ack()){
				TRY_IGNORE(set_scale(1));

				send_ps2_command(Ps2Command::enableReporting);
				TRY_IGNORE(read_ack());
			}

		}else if(rescale==2){
			send_ps2_command(Ps2Command::disableReporting);
			if(read_ack()){
				TRY_IGNORE(set_scale(2));

				send_ps2_command(Ps2Command::enableReporting);
				TRY_IGNORE(read_ack());
			}
		}
	}

	auto Ps2Mouse::get_position_x() -> I32 {
		return positionX;
	}

	auto Ps2Mouse::get_position_y() -> I32 {
		return positionY;
	}

	auto Ps2Mouse::get_button_count() -> U8 {
		return buttonCount;
	}

	auto Ps2Mouse::get_button_state(U8 button) -> bool {
		if(button>=buttonCount) return false;
		return buttonState[0];
	}

	void Ps2Mouse::send_ps2_command(Ps2Command command) {
		ps2->send_port_command(Ps2::Port::port2, (U8)command);
	}

	void Ps2Mouse::send_ps2_data(U8 data) {
		ps2->send_port_data(Ps2::Port::port2, data);
	}
}
