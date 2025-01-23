#include "exceptions.hpp"

#include <kernel/Cli.hpp>
#include <kernel/drivers.hpp>
#include <kernel/logging.hpp>
#include <kernel/PodArray.hpp>
#include <kernel/drivers/Interrupt.hpp>

#include <common/types.hpp>
#include <atomic>

namespace exceptions {
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
		void subscribe(U8 irq, irq::Subscriber callback, void *data) {
			auto &subscribers = irqSubscribers[irq];
			if(!subscribers){
				subscribers = new PodArray<IrqSubscription>(1);
				for(auto &driver:drivers::iterate<driver::Interrupt>()){
					if(irq>=driver.min_irq&&irq<=driver.max_irq){
						driver.enable_irq(0, irq); //TODO: multi-cpu?
					}
				}
			}
			subscribers->push_back(IrqSubscription{callback, data});
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
