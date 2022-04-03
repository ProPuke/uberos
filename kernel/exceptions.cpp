#include "exceptions.hpp"

#include <common/types.hpp>

#include <kernel/stdio.hpp>
#include <kernel/Cli.hpp>

#include <atomic>

namespace exceptions {
	// std::atomic<U32> _lock_depth = 0;
	volatile int _lock_depth = 0;

	Cli cli;

	void after_failure() {
		stdio::print_info("");
		stdio::print_info("Attempting to drop you into the backrooms...");
		stdio::print_info("");

		while(true){
			cli.prompt();
		}
	}
}
