#include "logging.hpp"

namespace logging {
	U32 indent;
	LList<Handler> handlers;

	// a circular history buffer. Always holds 2 null terminated strings, so that it can wrap wrap around (meaning the last byte must always be kept)
	char history[8192] = {};
	unsigned historyPosition = 0;

	void install_handler(Handler &handler) {
		handlers.push_back(handler);
	}

	void uninstall_handler(Handler &handler) {
		handlers.pop(handler);
	}

	namespace {
		inline void record(char c) {
			history[historyPosition++] = c;
			history[historyPosition] = '\0';

			if(historyPosition>=sizeof(history)-1) {
				history[0] = '\0';
				historyPosition = 0;
			}
		}
	}

	Handler historyHandler {
		[](U32 indent, logging::PrintType type) {
			while(indent--) {
				record(' ');
				record(' ');
			}
		},
		[](char c) {
			record(c);
		},
		[](const char *str) {
			for(;*str;str++){
				record(*str);
			}
		},
		[]() {
			record('\n');
		}
	};

	void init() {
		install_handler(historyHandler);
	}

	auto get_history_part_1() -> const char* {
		return &history[(historyPosition+1)%(sizeof(history)-1)]; // the string after where we're currently writing (the older part)
	}

	auto get_history_part_2() -> const char* {
		return &history[0]; // the string we're currently writing to (the new part)
	}
}
