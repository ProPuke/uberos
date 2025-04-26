#include "Apic.hpp"

#include <kernel/arch/x86/cpuInfo.hpp>
#include <kernel/arch/x86/msr.hpp>

namespace driver::system {
	namespace {
		auto check_supported() -> bool {
			return arch::x86::cpuInfo::get_features().apic;
		}
	}

	auto Apic::_on_start() -> Try<> {
		if(!check_supported()) return Failure{"Not supported by this CPU"};

		// arch::x86::msr::get();

		asm volatile(
			"mov ecx,0x1B\n"
			"rdmsr\n"
			"or	eax,0x800\n"
			"wrmsr\n"
		);

		return {};
	}

	auto Apic::_on_stop() -> Try<> {
		return {};
	}
}
