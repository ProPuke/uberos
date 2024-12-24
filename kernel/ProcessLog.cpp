#include "ProcessLog.hpp"

#include <kernel/logging.hpp>
#include <kernel/Process.hpp>

void ProcessLog::putc(char c) {
	logging::print_inline(c);
}

void ProcessLog::puts(const char *s) {
	logging::print_inline(s);
}

void ProcessLog::print_start(ProcessLogType type) {
	switch(type){
		case ProcessLogType::info:
			logging::print_info_start();
			puts(process.name);
			puts(":    ");
		break;
		case ProcessLogType::debug:
			logging::print_debug_start();
			puts(process.name);
			puts(":DBG ");
		break;
		case ProcessLogType::verbose:
			logging::print_info_start();
			puts(process.name);
			puts(":... ");
		break;
		case ProcessLogType::warning:
			logging::print_warning_start();
			puts(process.name);
			puts(":--- ");
		break;
		case ProcessLogType::error:
			logging::print_error_start();
			puts(process.name);
			puts(":!!! ");
		break;
	}
	for(U32 i=0;i<indent;i++){
		putc(' ');
		putc(' ');
	}
}

void ProcessLog::print_end() {
	logging::print_end();
}