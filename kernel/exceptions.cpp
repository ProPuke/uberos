#include "exceptions.hpp"

#include <kernel/Cli.hpp>
#include <kernel/drivers.hpp>
#include <kernel/logging.hpp>
#include <kernel/PodArray.hpp>

#include <common/types.hpp>
#include <atomic>

namespace exceptions {
	// std::atomic<U32> _lock_depth = 0;
	volatile int _lock_depth = 0;

	PodArray<irq::Subscriber*> *irqSubscribers[256] = {};

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
		void subscribe(U8 irq, irq::Subscriber callback) {
			auto &subscribers = irqSubscribers[irq];
			if(!subscribers){
				subscribers = new PodArray<irq::Subscriber*>(1);
			}
			subscribers->push_back(&callback);
		}

		void unsubscribe(U8 irq, irq::Subscriber callback) {
			auto &subscribers = irqSubscribers[irq];
			if(subscribers){
				for(auto i=0u;i<subscribers->length;i++){
					if((*subscribers)[i]==&callback){
						subscribers->remove(i);
						break;
					}
				}
			}
		}
	}

	void _on_irq(U8 irq) {
		auto &subscribers = irqSubscribers[irq];
		if(subscribers){
			for(auto subscriber:*subscribers){
				(*subscriber)(irq);
			}
		}

		drivers::_on_irq(irq);
	}
}
