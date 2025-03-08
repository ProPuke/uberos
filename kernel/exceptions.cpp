#include "exceptions.hpp"

#include <drivers/Interrupt.hpp>

#include <kernel/Cli.hpp>
#include <kernel/drivers.hpp>
#include <kernel/logging.hpp>

#include <common/PodArray.hpp>
#include <common/types.hpp>
#include <atomic>

namespace exceptions {
	volatile bool _enabled = false;
	// std::atomic<U32> _lock_depth = 0;
	volatile U32 _lock_depth = 0;

	struct IrqSubscription {
		irq::Subscriber subscriber;
		void *data;
	};

	PodArray<IrqSubscription> *irqSubscribers[256] = {};

	Cli cli;

	void after_failure() {
		logging::print_info("");
		logging::print_info("Dropping you into the backrooms...");
		logging::print_info("");

		while(true){
			cli.prompt();
		}
	}

	namespace irq {
		auto subscribe(U8 irq, irq::Subscriber callback, void *data) -> Try<> {
			auto &subscribers = irqSubscribers[irq];
			if(!subscribers){
				subscribers = new PodArray<IrqSubscription>(1);
				for(auto &driver:drivers::iterate<driver::Interrupt>()){
					if(irq>=driver.min_irq&&irq<=driver.max_irq){
						driver.enable_irq(0, irq); //TODO: multi-cpu?
						goto found;
					}
				}

				return {"IRQ not available"};

				found:
				;
			}

			subscribers->push_back(IrqSubscription{callback, data});

			return {};
		}

		void unsubscribe(U8 irq, irq::Subscriber callback, void *data) {
			auto &subscribers = irqSubscribers[irq];
			if(subscribers){
				for(auto i=0u;i<subscribers->length;i++){
					auto &subscription = (*subscribers)[i];
					if(subscription.subscriber==callback&&subscription.data==data){
						subscribers->remove(i);
						break;
					}
				}
				if(subscribers->length<1){
					delete subscribers;
					irqSubscribers[irq] = nullptr;
					for(auto &driver:drivers::iterate<driver::Interrupt>()){
						if(irq>=driver.min_irq&&irq<=driver.max_irq){
							driver.disable_irq(0, irq); //TODO: multi-cpu?
						}
					}
				}
			}
		}
	}

	void _on_irq(U8 irq) {
		auto subscribers = irqSubscribers[irq];
		if(subscribers){
			for(auto subscription:*subscribers){
				(*subscription.subscriber)(irq, subscription.data);
			}
		}

		drivers::_on_irq(irq);
	}
}
