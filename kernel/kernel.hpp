#pragma once

namespace kernel {
	void init(void(*preinit)(), void(*init)(), void(*postinit)());
}
