#pragma once

#include <kernel/arch/x86/PciDevice.hpp>
#include <kernel/PodArray.hpp>
#ifdef ARCH_X86
	#include <kernel/arch/x86/ioPort.hpp>
#endif

#include <common/Bitmask.hpp>
#include <common/Try.hpp>
#include <common/types.hpp>

struct Driver;

namespace drivers{
	void print_driver_summary(const char *indent, Driver&);
}

class DriverApi {
	public:
		enum struct Startup {
			disabled,
			onDemand,
			automatic,
			max
		};

		enum struct State {
			inactive,
			active,
			failed,
			max
		};

	protected:
		friend auto to_string(DriverApi::State) -> const char*;
		friend auto to_string(DriverApi::Startup) -> const char*;
		friend void drivers::print_driver_summary(const char *indent, Driver&);

		static inline const char *startup_name[(U64)Startup::max+1] = {
			"disabled",
			"on-demand",
			"automatic"
		};

		static inline const char *state_name[(U64)State::max+1] = {
			"inactive",
			"active",
			"failed"
		};

		Startup startup;
		State state = State::inactive;

		struct MemoryRange {
			void *start;
			void *end;
		};

		Bitmask256 subscribedIrqs;
		Bitmask256 subscribedInterrupts;
		bool       subscribedAllInterrupts = false;
		PodArray<MemoryRange> subscribedMemory;
		PodArray<PciDevice*> subscribedPciDevices;
		#ifdef ARCH_X86
			PodArray<arch::x86::IoPort> subscribedIoPorts;
		#endif

	public:
		/**/ DriverApi(Startup startup):
			startup(startup)
		{}

		auto driver() -> Driver&;

		// cpu interrupts
		void subscribe_interrupt(U8);
		void unsubscribe_interrupt(U8);
		void unsubscribe_all_interrupts();

		// hardware irqs (before routed to possibly different interrupts)
		auto subscribe_irq(U8) -> Try<>;
		auto subscribe_available_irq() -> Try<U8> { return subscribe_available_irq({~(U64)0,~(U64)0,~(U64)0,~(U64)0}); }
		auto subscribe_available_irq(Bitmask256) -> Try<U8>;
		void unsubscribe_irq(U8);
		void unsubscribe_all_irqs();

		auto subscribe_memory(void*, size_t) -> Try<>;
		void unsubscribe_memory(void*, size_t);
		void unsubscribe_all_memory();
		auto is_subscribed_to_memory(void*, size_t) -> bool;

		auto subscribe_pci(PciDevice&) -> Try<>;
		void unsubscribe_pci(PciDevice&);
		void unsubscribe_all_pci();
		auto is_subscribed_to_pci(PciDevice&) -> bool;

		#ifdef ARCH_X86
			auto subscribe_ioPort(arch::x86::IoPort) -> Try<>;
			void unsubscribe_ioPort(arch::x86::IoPort);
			void unsubscribe_all_ioPort();
			auto is_subscribed_to_ioPort(arch::x86::IoPort) -> bool;
		#endif

		auto start_driver() -> Try<>;
		auto stop_driver() -> Try<>;
		auto restart_driver() -> Try<>;
		void fail_driver(const char *reason);

		auto is_enabled() -> bool { return startup!=Startup::disabled; }
		auto is_automatic() -> bool { return startup==Startup::automatic; }
		auto is_disabled() -> bool { return startup==Startup::disabled; }
		auto is_active() -> bool { return state==State::active; }
		auto is_failed() -> bool { return state==State::failed; }
};

inline auto to_string(DriverApi::State state) -> const char* { return DriverApi::state_name[(U64)state]; }
inline auto to_string(DriverApi::Startup startup) -> const char* { return DriverApi::startup_name[(U64)startup]; }
