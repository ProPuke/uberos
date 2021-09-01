#pragma once
#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/ThreadCpuState.hpp>

#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/ThreadCpuState.hpp>
	
#else
	#error "Unsupported architecture"
#endif