#include "exceptions.hpp"

#include <common/types.hpp>

#include <kernel/log.hpp>
#include <kernel/Cli.hpp>

#include <atomic>

namespace exceptions {
	// std::atomic<U32> _lock_depth = 0;
	volatile int _lock_depth = 0;

	Cli cli;

	void after_failure() {
		log::print_info("");
		log::print_info("Dropping you into the backrooms...");
		log::print_info("");

		while(true){
			cli.prompt();
		}
	}
}
