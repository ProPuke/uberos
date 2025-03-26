#include "X86.hpp"

#include <kernel/arch/x86/cpuInfo.hpp>

namespace driver::processor {
	namespace {
		volatile void *localApic = nullptr;
	}

	auto X86::_on_start() -> Try<> {
		if(::processor::driver&&::processor::driver!=this) return {"A CPU driver is already active"};

		arch::x86::cpuInfo::get_vendor_string(vendorStringData);

		#ifdef ARCH_X86_64
			processor_arch = "x86-64";
		#else
			processor_arch = "x86-32";
		#endif

		auto features = arch::x86::cpuInfo::get_features();

		log.print_info_start();
		log.print_inline("Features:");

		if(features.fpu) log.print_inline(" fpu");
		if(features.vme) log.print_inline(" vme");
		if(features.de) log.print_inline(" de");
		if(features.pse) log.print_inline(" pse");
		if(features.tsc) log.print_inline(" tsc");
		if(features.msr) log.print_inline(" msr");
		if(features.pae) log.print_inline(" pae");
		if(features.mce) log.print_inline(" mce");
		if(features.cx8) log.print_inline(" cx8");
		if(features.apic) log.print_inline(" apic");
		if(features.sep) log.print_inline(" sep");
		if(features.mtrr) log.print_inline(" mtrr");
		if(features.pge) log.print_inline(" pge");
		if(features.mca) log.print_inline(" mca");
		if(features.cmov) log.print_inline(" cmov");
		if(features.pat) log.print_inline(" pat");
		if(features.pse36) log.print_inline(" pse36");
		if(features.psn) log.print_inline(" psn");
		if(features.clflush) log.print_inline(" clflush");
		if(features.ds) log.print_inline(" ds");
		if(features.acpi) log.print_inline(" acpi");
		if(features.mmx) log.print_inline(" mmx");
		if(features.fxsr) log.print_inline(" fxsr");
		if(features.sse) log.print_inline(" sse");
		if(features.sse2) log.print_inline(" sse2");
		if(features.ss) log.print_inline(" ss");
		if(features.htt) log.print_inline(" htt");
		if(features.tm) log.print_inline(" tm");
		if(features.ia64) log.print_inline(" ia64");
		if(features.pbe) log.print_inline(" pbe");

		if(features.sse3) log.print_inline(" sse3");
		if(features.pclmul) log.print_inline(" pclmul");
		if(features.dtes64) log.print_inline(" dtes64");
		if(features.monitor) log.print_inline(" monitor");
		if(features.ds_cpl) log.print_inline(" ds_cpl");
		if(features.vmx) log.print_inline(" vmx");
		if(features.smx) log.print_inline(" smx");
		if(features.est) log.print_inline(" est");
		if(features.tm2) log.print_inline(" tm2");
		if(features.ssse3) log.print_inline(" ssse3");
		if(features.cid) log.print_inline(" cid");
		if(features.sdbg) log.print_inline(" sdbg");
		if(features.fma) log.print_inline(" fma");
		if(features.cx16) log.print_inline(" cx16");
		if(features.xtpr) log.print_inline(" xtpr");
		if(features.pdcm) log.print_inline(" pdcm");
		if(features.pcid) log.print_inline(" pcid");
		if(features.dca) log.print_inline(" dca");
		if(features.sse4_1) log.print_inline(" sse4_1");
		if(features.sse4_2) log.print_inline(" sse4_2");
		if(features.x2apic) log.print_inline(" x2apic");
		if(features.movbe) log.print_inline(" movbe");
		if(features.popcnt) log.print_inline(" popcnt");
		if(features.tsc) log.print_inline(" tsc");
		if(features.aes) log.print_inline(" aes");
		if(features.xsave) log.print_inline(" xsave");
		if(features.osxsave) log.print_inline(" osxsave");
		if(features.avx) log.print_inline(" avx");
		if(features.f16c) log.print_inline(" f16c");
		if(features.rdrand) log.print_inline(" rdrand");
		if(features.hypervisor) log.print_inline(" hypervisor");

		log.print_end();

		localApic = TRY_RESULT(api.subscribe_memory(Physical<void>{0xfee00000}, memory::pageSize, mmu::Caching::uncached));

		::processor::driver = this;

		return {};
	}

	auto X86::get_active_id() -> U32 {
		const auto lapic_id = 0x20;
		return *(volatile U32*)((U8*)localApic+lapic_id) >> 24;
	}
}
