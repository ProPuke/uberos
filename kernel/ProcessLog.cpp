#include "ProcessLog.hpp"

#include "Process.hpp"
#include <kernel/stdio.hpp>

void ProcessLog::putc(char c) {
	stdio::print_inline(c);
}

void ProcessLog::puts(const char *s) {
	stdio::print_inline(s);
}

void ProcessLog::print_start(ProcessLogType type) {
	switch(type){
		case ProcessLogType::info:
			stdio::print_info_start();
			puts(process.name);
			puts(":    ");
		break;
		case ProcessLogType::debug:
			stdio::print_info_start();
			puts(process.name);
			puts(":DBG ");
		break;
		case ProcessLogType::verbose:
			stdio::print_info_start();
			puts(process.name);
			puts(":... ");
		break;
		case ProcessLogType::warning:
			stdio::print_warning_start();
			puts(process.name);
			puts(":--- ");
		break;
		case ProcessLogType::error:
			stdio::print_error_start();
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
	stdio::print_end();
}