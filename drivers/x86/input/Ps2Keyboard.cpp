#include "Ps2Keyboard.hpp"

#include <drivers/x86/system/Ps2.hpp>

#include <kernel/DriverReference.hpp>
#include <kernel/panic.hpp>
#include <kernel/keyboard.hpp>

namespace driver::input {
	namespace {
		typedef system::Ps2 Ps2;

		using keyboard::Scancode;
		using keyboard::ScancodeUk;

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

		void trigger_event(Keyboard::Event event) {
			event.instance = &Ps2Keyboard::instance;

			if(event.type==Ps2Keyboard::Event::Type::pressed){
				const auto shift = Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::leftShift)||Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::rightShift);
				const auto control = Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::leftControl)||Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::rightControl);
				const auto alt = Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::alt);
				const auto super = Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::leftSuper)||Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::rightSuper);

				if(control&&event.pressed.scancode!=(Scancode)ScancodeUk::leftControl&&event.pressed.scancode!=(Scancode)ScancodeUk::rightControl){
					event.pressed.modifiers |= (U8)Keyboard::Modifier::control;
				}
				if(alt&&event.pressed.scancode!=(Scancode)ScancodeUk::alt){
					event.pressed.modifiers |= (U8)Keyboard::Modifier::alt;
				}
				if(super&&event.pressed.scancode!=(Scancode)ScancodeUk::leftSuper&&event.pressed.scancode!=(Scancode)ScancodeUk::rightSuper){
					event.pressed.modifiers |= (U8)Keyboard::Modifier::super;
				}
				// shift is only marked as a modifier if other modifiers are also pressed (otherwise it's just a shift+type)
				if(shift&&event.pressed.modifiers&&event.pressed.scancode!=(Scancode)ScancodeUk::leftShift&&event.pressed.scancode!=(Scancode)ScancodeUk::rightShift){
					event.pressed.modifiers |= (U8)Keyboard::Modifier::shift;
				}
			}

			Ps2Keyboard::instance.events.trigger(event);
			Ps2Keyboard::allEvents.trigger(event);

			if(event.type==Ps2Keyboard::Event::Type::pressed&&!event.pressed.modifiers){
				const auto shift = Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::leftShift)||Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::rightShift);
				const auto altGr = Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::altGr);

				auto layout = Ps2Keyboard::instance.layout;
				auto map = altGr&&shift?layout->charmapShiftAltGr:altGr?layout->charmapAltGr:shift?layout->charmapShift:layout->charmap;

				if(auto character = map[event.pressed.scancode]){
					trigger_event({
						type: Ps2Keyboard::Event::Type::characterTyped,
						characterTyped: { character }
					});
				}
			}
		}

		Bitmask256 keystate;
	}

	auto Ps2Keyboard::_on_start() -> Try<> {
		ps2 = drivers::find_and_activate<system::Ps2>(this);
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

		// write_command(Command::write_config);
		// write_data(config&~((U8)Config::first_irq_mask|(U8)Config::second_irq_mask|(U8)Config::translation));

		U8 scancode = TRY_RESULT(get_scancode());
		log.print_info("scancode: ", scancode);

		TRY(set_repeat());

		// send_ps2_command(Ps2Command::enableReporting);
		// TRY(read_ack());

		if(!ps2->_install_port_device(Ps2::Port::port1, *this)) return {"Unable to install PS/2 port 1 device"};
		
		TRY(api.subscribe_irq(irq));

		{ // enable irqs
			ps2->write_command(Ps2::Command::read_config);
			auto config = ps2->read_data();

			ps2->write_command(Ps2::Command::write_config);
			ps2->write_data(config|(U8)Ps2::Config::first_irq_mask|(U8)Ps2::Config::translation);
		}

		return {};
	}

	auto Ps2Keyboard::_on_stop() -> Try<> {
		if(ps2){
			ps2->_uninstall_port_device(Ps2::Port::port1, *this);
		}

		return {};
	}

	namespace {
		static U8 readBuffer[6];
		static U8 readBufferPosition = 0;

		auto process_buffer() {
			while(readBufferPosition>0){
				// Ps2Keyboard::instance.log.print_info_start();
				// Ps2Keyboard::instance.log.print_inline("buffer: ");
				// for(auto i=0;i<readBufferPosition;i++){
				// 	Ps2Keyboard::instance.log.print_inline(format::Hex8{readBuffer[i]}, ' ');
				// }
				// Ps2Keyboard::instance.log.print_end();

				if(readBuffer[0]==0xe0){
					// 2 bytes needed
					auto readSize = 2;

					if(readBufferPosition<2) return;

					U16 code = readBuffer[0]<<8|readBuffer[1];
					auto isDown = !(code&0b10000000);
					code &=code&0b1111111101111111;

					Scancode scancode = 0;
					const char *action = nullptr;

					switch(code&0xff){
						case 0x1c: scancode = (Scancode)ScancodeUk::numpadEnter; break;
						case 0x1d: scancode = (Scancode)ScancodeUk::rightControl; break;
						case 0x10: action = "mediaTrackPrevious"; break;
						case 0x19: action = "mediaTrackNext"; break;
						case 0x20: action = "volumeMute"; break;
						case 0x21: action = "launchCalculator"; break;
						case 0x22: action = "mediaPlayPause"; break;
						case 0x24: action = "mediaStop"; break;
						case 0x2a: // fake left shift. These appear alongside some other keys for old hacksy IBM AT backward compat. They can be ignored.
							memcpy(&readBuffer[0], &readBuffer[readSize], readBufferPosition-=readSize);
							continue;
						break; 
						case 0x2e: action = "volumeDown"; break;
						case 0x30: action = "volumeUp"; break;
						case 0x32: action = "webHome"; break;
						case 0x65: action = "webSearch"; break;
						case 0x66: action = "webFavourites"; break;
						case 0x67: action = "webRefresh"; break;
						case 0x68: action = "webStop"; break;
						case 0x69: action = "webForward"; break;
						case 0x6a: action = "webBack"; break;
						case 0x6b: action = "launchComputer"; break;
						case 0x6c: action = "launchEmail"; break;
						case 0x6d: action = "mediaSelect"; break;
						case 0x35: scancode = (Scancode)ScancodeUk::numpadDivide; break;
						case 0x37: scancode = (Scancode)ScancodeUk::printScreen; break; // with ctrl held
						case 0x38: scancode = (Scancode)ScancodeUk::altGr; break;
						case 0x46: scancode = (Scancode)ScancodeUk::scrollLock; break; //only triggered when ctrl is also held, so technically ctrl+break
						case 0x47: scancode = (Scancode)ScancodeUk::home; break;
						case 0x48: scancode = (Scancode)ScancodeUk::upArrow; break;
						case 0x49: scancode = (Scancode)ScancodeUk::pageUp; break;
						case 0x4b: scancode = (Scancode)ScancodeUk::leftArrow; break;
						case 0x4d: scancode = (Scancode)ScancodeUk::rightArrow; break;
						case 0x4f: scancode = (Scancode)ScancodeUk::end; break;
						case 0x50: scancode = (Scancode)ScancodeUk::downArrow; break;
						case 0x51: scancode = (Scancode)ScancodeUk::pageDown; break;
						case 0x52: scancode = (Scancode)ScancodeUk::insert; break;
						case 0x53: scancode = (Scancode)ScancodeUk::_delete; break;
						case 0x5b: scancode = (Scancode)ScancodeUk::leftSuper; break;
						case 0x5c: scancode = (Scancode)ScancodeUk::rightSuper; break;
						case 0x5d: scancode = (Scancode)ScancodeUk::contextMenu; break;
					}

					// 0xe0 0x37 - ctrl+printscreen press (0)
					// 0xe0 0xb7 - ctrl+printscreen release (1)
					// 0xe0 0x2a 0xe0 0x37 - printscreen press (0)
					// 0xe0 0xb7 0xe0 0xaa - printscreen release (1)
					
					memcpy(&readBuffer[0], &readBuffer[readSize], readBufferPosition-=readSize);

					if(!scancode&&!action){
						Ps2Keyboard::instance.log.print_warning("Unsupported keycode: ", format::Hex16{code}, isDown?" (pressed)":" (released)");
						continue;
					}

					if(scancode){
						keystate.set(scancode, isDown);
						if(isDown){
							trigger_event({
								type: Ps2Keyboard::Event::Type::pressed,
								pressed: { scancode, false }
							});

						}else{
							trigger_event({
								type: Ps2Keyboard::Event::Type::released,
								pressed: { scancode }
							});
						}
					}

					if(action){
						if(isDown){
							trigger_event({
								type: Ps2Keyboard::Event::Type::actionPressed,
								actionPressed: { action }
							});
						}
					}

				}else if(readBuffer[0]==0xe1){
					// 2 bytes needed
					if(readBufferPosition<2) return;

					U16 code = readBuffer[0]<<8|readBuffer[1];
					auto isDown = !(code&0b10000000);
					code &=code&0b1111111101111111;

					auto readSize = 2;

					Scancode scancode = 0;
					switch(code&0xff){
						case 0x1d:
							if(readBufferPosition<6) return;

							readSize = 6;

							if(isDown&&readBuffer[2]==0x45&&readBuffer[3]==0xe1&&readBuffer[4]==0x9d&&readBuffer[5]==0xc5){
								scancode = (Scancode)ScancodeUk::scrollLock;

								// this has no key release event, so manually trigger a press event here..
								keystate.set(scancode, isDown);
								trigger_event({
									type: Ps2Keyboard::Event::Type::pressed,
									pressed: { scancode, false }
								});

								// and then trigger a release next
								isDown = false;
							}else{
								scancode = 0;
							}
						break;
					}

					memcpy(&readBuffer[0], &readBuffer[readSize], readBufferPosition-=readSize);

					if(!scancode){
						Ps2Keyboard::instance.log.print_warning("Unsupported keycode: ", format::Hex16{code}, isDown?" (pressed)":" (released)");
						continue;
					}

					keystate.set(scancode, isDown);
					if(isDown){
						trigger_event({
							type: Ps2Keyboard::Event::Type::pressed,
							pressed: { scancode, false }
						});

					}else{
						trigger_event({
							type: Ps2Keyboard::Event::Type::released,
							pressed: { scancode }
						});
					}

				}else{
					// 1 byte needed
					auto code = readBuffer[0];
					auto isDown = !(code&0b10000000);
					code &=code&0b01111111;

					memcpy(&readBuffer[0], &readBuffer[1], readBufferPosition-=1);

					Scancode translation[0x6c] = {
						0, (Scancode)ScancodeUk::escape,
						(Scancode)ScancodeUk::number1, (Scancode)ScancodeUk::number2, (Scancode)ScancodeUk::number3, (Scancode)ScancodeUk::number4, (Scancode)ScancodeUk::number5, (Scancode)ScancodeUk::number6, (Scancode)ScancodeUk::number7, (Scancode)ScancodeUk::number8, (Scancode)ScancodeUk::number9, (Scancode)ScancodeUk::number0, (Scancode)ScancodeUk::hyphen, (Scancode)ScancodeUk::equals, (Scancode)ScancodeUk::backspace,
						(Scancode)ScancodeUk::tab, (Scancode)ScancodeUk::letterQ, (Scancode)ScancodeUk::letterW, (Scancode)ScancodeUk::letterE, (Scancode)ScancodeUk::letterR, (Scancode)ScancodeUk::letterT, (Scancode)ScancodeUk::letterY, (Scancode)ScancodeUk::letterU, (Scancode)ScancodeUk::letterI, (Scancode)ScancodeUk::letterO, (Scancode)ScancodeUk::letterP, (Scancode)ScancodeUk::leftBracket, (Scancode)ScancodeUk::rightBracket, (Scancode)ScancodeUk::enter,
						(Scancode)ScancodeUk::leftControl,
						(Scancode)ScancodeUk::letterA, (Scancode)ScancodeUk::letterS, (Scancode)ScancodeUk::letterD, (Scancode)ScancodeUk::letterF, (Scancode)ScancodeUk::letterG, (Scancode)ScancodeUk::letterH, (Scancode)ScancodeUk::letterJ, (Scancode)ScancodeUk::letterK, (Scancode)ScancodeUk::letterL, (Scancode)ScancodeUk::semicolon, (Scancode)ScancodeUk::apostrophe, (Scancode)ScancodeUk::backTick,
						(Scancode)ScancodeUk::leftShift, (Scancode)ScancodeUk::hash, (Scancode)ScancodeUk::letterZ, (Scancode)ScancodeUk::letterX, (Scancode)ScancodeUk::letterC, (Scancode)ScancodeUk::letterV, (Scancode)ScancodeUk::letterB, (Scancode)ScancodeUk::letterN, (Scancode)ScancodeUk::letterM, (Scancode)ScancodeUk::comma, (Scancode)ScancodeUk::dot, (Scancode)ScancodeUk::slash, (Scancode)ScancodeUk::rightShift,
						(Scancode)ScancodeUk::numpadMultiply,
						(Scancode)ScancodeUk::alt, (Scancode)ScancodeUk::space, (Scancode)ScancodeUk::capslock,
						(Scancode)ScancodeUk::f1, (Scancode)ScancodeUk::f2, (Scancode)ScancodeUk::f3, (Scancode)ScancodeUk::f4, (Scancode)ScancodeUk::f5, (Scancode)ScancodeUk::f6, (Scancode)ScancodeUk::f7, (Scancode)ScancodeUk::f8, (Scancode)ScancodeUk::f9, (Scancode)ScancodeUk::f10,
						(Scancode)ScancodeUk::numLock, (Scancode)ScancodeUk::_pause,
						(Scancode)ScancodeUk::numpad7, (Scancode)ScancodeUk::numpad8, (Scancode)ScancodeUk::numpad9, (Scancode)ScancodeUk::numpadSubtract, (Scancode)ScancodeUk::numpad4, (Scancode)ScancodeUk::numpad5, (Scancode)ScancodeUk::numpad6, (Scancode)ScancodeUk::numpadPlus, (Scancode)ScancodeUk::numpad1, (Scancode)ScancodeUk::numpad2, (Scancode)ScancodeUk::numpad3, (Scancode)ScancodeUk::numpad0, (Scancode)ScancodeUk::numpadDot,
						(Scancode)ScancodeUk::printScreen, // alt-sysrq
						0, //0x55
						(Scancode)ScancodeUk::backslash,
						(Scancode)ScancodeUk::f11, (Scancode)ScancodeUk::f12, 0, 0, (Scancode)ScancodeUk::f13, (Scancode)ScancodeUk::f14, (Scancode)ScancodeUk::f15, 0, 0, 0, 0, 0, (Scancode)ScancodeUk::f16, (Scancode)ScancodeUk::f17, (Scancode)ScancodeUk::f18, (Scancode)ScancodeUk::f19, (Scancode)ScancodeUk::f20, (Scancode)ScancodeUk::f21, (Scancode)ScancodeUk::f22, (Scancode)ScancodeUk::f23, (Scancode)ScancodeUk::f24
					};

					if(code>sizeof(translation)||!translation[code]){
						Ps2Keyboard::instance.log.print_warning("Unsupported keycode: ", format::Hex8{code}, isDown?" (pressed)":" (released)");
						continue;
					}

					auto scancode = translation[code];
					keystate.set(scancode, isDown);

					if(isDown&&(ScancodeUk)scancode==ScancodeUk::_pause){
						const auto control = Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::leftControl)||Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::rightControl);
						const auto alt = Ps2Keyboard::instance.is_pressed((Scancode)ScancodeUk::alt);
						if(control&&alt){
							panic::panic().print_details("Keyboard interrupt - CTRL+ALT+BREAK");
						}
					}

					if(isDown){
						trigger_event({
							type: Ps2Keyboard::Event::Type::pressed,
							pressed: { scancode, false }
						});

					}else{
						trigger_event({
							type: Ps2Keyboard::Event::Type::released,
							pressed: { scancode }
						});
					}
				}
			}
		}
	}

	void Ps2Keyboard::_on_irq(U8 _irq) {
		if(_irq!=irq) return;

		while(Ps2::instance.has_data()){
			auto status = ps2->read_status();
			if((U8)status&(U8)Ps2::Status::mouse_byte) return;
			readBuffer[readBufferPosition++] = ps2->read_data();
			// auto count = readBufferPosition;
			process_buffer();

			// log.print_info(count-readBufferPosition, " bytes read. ", readBufferPosition, " bytes still in the buffer");

			if(readBufferPosition==sizeof(readBuffer)){
				// we have too much! clear the buffer and try again..
				readBufferPosition = 0;
			}
		}
	}

	auto Ps2Keyboard::is_pressed(keyboard::Scancode scancode) -> bool {
		return keystate.get(scancode);
	}

	void Ps2Keyboard::send_ps2_command(Ps2Command command) {
		ps2->send_port_command(Ps2::Port::port1, (U8)command);
	}

	void Ps2Keyboard::send_ps2_data(U8 data) {
		ps2->send_port_data(Ps2::Port::port1, data);
	}
}
