#pragma once

namespace driver {
	struct Processor;
}

namespace processor {
	extern driver::Processor *driver;

	auto get_active_id() -> U32;

	void pause();
}

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/processor.hpp>
	
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/processor.hpp>

#elif defined(ARCH_X86)
	#include <kernel/arch/x86/processor.hpp>
#endif
