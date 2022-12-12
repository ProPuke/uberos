#include "ProcessLog.hpp"

#include "Process.hpp"
#include <kernel/log.hpp>

void ProcessLog::putc(char c) {
	log::print_inline(c);
}

void ProcessLog::puts(const char *s) {
	log::print_inline(s);
}

void ProcessLog::print_start(ProcessLogType type) {
	switch(type){
		case ProcessLogType::info:
			log::print_info_start();
			puts(process.name);
			puts(":    ");
		break;
		case ProcessLogType::debug:
			log::print_debug_start();
			puts(process.name);
			puts(":DBG ");
		break;
		case ProcessLogType::verbose:
			log::print_info_start();
			puts(process.name);
			puts(":... ");
		break;
		case ProcessLogType::warning:
			log::print_warning_start();
			puts(process.name);
			puts(":--- ");
		break;
		case ProcessLogType::error:
			log::print_error_start();
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
	log::print_end();
}