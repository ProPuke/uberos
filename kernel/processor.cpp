#include "processor.hpp"

#include <drivers/Processor.hpp>

namespace processor {
	driver::Processor *driver = nullptr;

	auto get_active_id() -> U32 { return driver?driver->get_active_id():0; }
}
