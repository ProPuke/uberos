#include "log.hpp"

namespace log {
	U32 indent;
	LList<Handler> handlers;

	void install_handler(Handler &handler) {
		handlers.push_back(handler);
	}

	void uninstall_handler(Handler &handler) {
		handlers.pop(handler);
	}
}
