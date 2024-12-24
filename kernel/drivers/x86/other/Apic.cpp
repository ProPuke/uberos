#include "Apic.hpp"

#include <kernel/arch/x86/cpuInfo.hpp>
#include <kernel/arch/x86/msr.hpp>

namespace driver {
	namespace {
		auto check_supported() -> bool {
			return arch::x86::cpuInfo::get_features().apic;
		}
	}

	namespace processor {
		DriverType Apic::driverType{"APIC", &Super::driverType};

		/**/ Apic::Apic():
			Driver("APIC", "APIC Interupt Controller")
		{
			type = &driverType;
		}

		auto Apic::_on_start() -> bool {
			if(!check_supported()) return false;

			// arch::x86::msr::get();

			return true;
		}

		auto Apic::_on_stop() -> bool {
			return true;
		}
	}
}
