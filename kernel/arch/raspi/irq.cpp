#include "irq.hpp"

#include "mmio.hpp"
#include "timer.hpp"

#include <kernel/exceptions.hpp>
#include <kernel/device.hpp>
#include <kernel/stdio.hpp>
#include <cstddef>

namespace mmio {
	using namespace arch::raspi;
}

namespace timer {
	using namespace arch::raspi;
}

namespace irq {
	namespace arch {
		namespace raspi {
			extern "C" void handle_interrupt() {
				// stdio::print_debug("irq?");

				if(interruptController.state!=Driver::State::enabled) return;

				exceptions::Guard guard;
				// stdio::print_debug("irq");

				interruptController.handle_interrupt([](U32 irq) {
					// stdio::print_debug("irq ", irq);

					switch((Irq)irq){
						case Irq::system_timer_0:
							timer::on_interrupt(timer::Timer::gpu0);
						break;
						case Irq::system_timer_1:
							timer::on_interrupt(timer::Timer::cpu_scheduler);
						break;
						case Irq::system_timer_2:
							timer::on_interrupt(timer::Timer::gpu1);
						break;
						case Irq::system_timer_3:
							timer::on_interrupt(timer::Timer::cpu_slow_scheduler);
						break;
						// case 0x60:
						// 	timer::on_interrupt(timer::Timer::gpu0);
						// break;
						// case 0x61:
						// 	timer::on_interrupt(timer::Timer::cpu_scheduler);
						// break;
						// case 0x62:
						// 	timer::on_interrupt(timer::Timer::gpu1);
						// break;
						// case 0x63:
						// 	timer::on_interrupt(timer::Timer::cpu_slow_scheduler);
						// break;
						case Irq::usb_controller:
						break;
						case Irq::hdmi_0:
						case Irq::hdmi_1:
						break;
						case Irq::arm_timer:
						break;
					}
				});
			}

			void init() {
				stdio::Section section("irq::arch::raspi::init...");

				device::install_device(interruptController, true);
			}

			void enable(Irq irq) {
				interruptController.enable_irq(0, (U32)irq);
			}

			void disable(Irq irq) {
				interruptController.disable_irq(0, (U32)irq);
			}
		}
	}
}
